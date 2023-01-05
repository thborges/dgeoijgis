package peer

/*
#cgo CFLAGS: -std=gnu11
#include <stdlib.h>
#include <resource.h>
*/
import "C"

import (
	"sync"
	"time"
	"log"
	"unsafe"
	"os"
)

var Flog = log.New(os.Stderr, "Fatal: ", log.Ltime | log.Lshortfile)

type Envelope struct {
	MinX float64
	MinY float64
	MaxX float64
	MaxY float64
}

type SegmentId struct {
	X uint16
	Y uint16
}

func (s *SegmentId) GetCSid() uint32 {
	return uint32(s.X) + uint32(s.Y)<<16
}

func (s *SegmentId) IsDiff(d SegmentId) bool {
	return s.X != d.X || s.Y != d.Y
}

type RequestMsg struct {
	RType RequestType
}

type NewDataset struct {
	Dataset  string
	Metadata []byte
	GeoCount int
	Intermed bool
}

type LoadMsg struct {
	Sid    SegmentId
	Wkb    []byte
	Mbr    Envelope
	Gid    int64
	IsCopy bool
	End    bool
}

type CopyItemMsg struct {
	WKT []string
	Gid int64
	End bool
	Card int
}

type DatasetSeg struct {
	Host    string
	Dataset string
	Sid     SegmentId
	copied  bool
}

func (c *DatasetSeg) IsDiff(d DatasetSeg) bool {
	return c.Sid.X != d.Sid.X || c.Sid.Y != d.Sid.Y || c.Dataset != d.Dataset || c.Host != d.Host
}

type JoinMsg struct {
	Level uint8
	L     DatasetSeg
	R     []DatasetSeg
	End   bool
}

type JoinMsgWrap struct {
	jm      *JoinMsg
	copycnt int
	copymtx *sync.Mutex
}

type ServerChan struct {
	copyChan  chan JoinMsgCopyWrap
	tocompare []chan *JoinMsg
	done      chan bool
}

type JoinStartMsg struct {
	QueryName string
	Levels uint8
}

type InstantReturn struct {
	AtTime  time.Time
	Results uint32
}

type InstantStepsReturn struct {
	m        sync.Mutex
	Instants []InstantReturn
}

type ResourceUsage struct {
	AtTime     time.Time
	User_cpu   float64
	System_cpu float64
	Net_in     uint32
	Net_out    uint32
	Pred_queue uint32
	Copy_queue uint32
}

type JoinEndMsg struct {
	StepsReturns      []InstantStepsReturn
	ResultCardinality []uint32
	FirstUsage        ResourceUsage
	Usage             []ResourceUsage
}

type DatasetMetadata struct {
	Name     string
	Srs      string
	GeoCount int
	BBox     Envelope
}

type GetDatasetsMsg struct {
	Dts []DatasetMetadata
}

type GetDatasetsMeta struct {
	Dts []NewDataset
}

func (jem *JoinEndMsg) getUsage(intf string, copiers map[string]ServerChan, tocompare []chan *JoinMsg) ResourceUsage {
	cpu := C.get_cpu_usage()

	cintf := C.CString(intf)
	net := C.get_net_usage(cintf)
	C.free(unsafe.Pointer(cintf))

	pred := 0
	for _, cq := range tocompare {
		pred += len(cq)
	}
	cpy := 0
	for _, serverchan := range copiers {
		cpy += len(serverchan.copyChan)
	}

	return ResourceUsage{time.Now().Local(),
		float64(cpu.user_ms) - jem.FirstUsage.User_cpu,
		float64(cpu.system_ms) - jem.FirstUsage.System_cpu,
		uint32(net.net_in) - jem.FirstUsage.Net_in,
		uint32(net.net_out) - jem.FirstUsage.Net_out,
		uint32(pred), uint32(cpy)}
}

func (jem *JoinEndMsg) RegisterUsage(intf string, copiers map[string]ServerChan, tocompare []chan *JoinMsg) {
	if cap(jem.Usage) == 0 {
		jem.FirstUsage = jem.getUsage(intf, copiers, tocompare[0:])
		jem.Usage = make([]ResourceUsage, 0, 1)
	} else {
		jem.Usage = append(jem.Usage, jem.getUsage(intf, copiers, tocompare[0:]))
	}
}

type CopyMsg struct {
	Dataset string
	Sid     SegmentId
	Filter	bool
	BBox	Envelope
}

type CleanMsg struct {
	End bool
}

type CleanIntermediateMsg struct {
	End bool
}

type StoreFolderMsg struct {
	Folder string
}

