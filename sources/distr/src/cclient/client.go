package main

/*
#cgo pkg-config: gdal
#cgo CFLAGS: -std=gnu11 -I../../../ -I../../../utils/
#cgo LDFLAGS: -L${SRCDIR}/../../../utils -lrtreeutils -lgeos -lgeos_c -lstdc++ -lm -lglpk
#include "system.h"
*/
import "C"

import (
	"encoding/gob"
	"log"
	"net"
	"os"
	"dgeodistr/peer"
	"time"
	"unsafe"
)

const (
	udp_group = "224.0.0.1:4567"
)

var (
	ap  peer.ActivePeers
	aps map[string]byte
)

func connect(url string) (*gob.Encoder, *gob.Decoder) {
	conn, err := net.Dial("tcp", url)
	if err != nil {
		log.Println(err)
		return nil, nil
	}
	return gob.NewEncoder(conn), gob.NewDecoder(conn)
}

func checkDispatch(err error) bool {
	if err != nil {
		log.Println(err)
		return false
	}
	return true
}

//export ActivePeersCount
func ActivePeersCount() int {
	return ap.ActivePeersCount()
}

//export Clean
func Clean() {
	req := peer.RequestMsg{}
	req.RType = peer.RClean

	for p, _ := range aps {
		enc, dec := connect(p)
		if enc != nil {
			enc.Encode(req)
			cm := peer.CleanMsg{}
			dec.Decode(&cm)
		}
	}
	C.clean_local_datasets()
}

//export CleanIntermediate
func CleanIntermediate() {
	req := peer.RequestMsg{}
	req.RType = peer.RCleanIntermediate

	for p, _ := range aps {
		enc, dec := connect(p)
		if enc != nil {
			enc.Encode(req)
			cm := peer.CleanIntermediateMsg{}
			dec.Decode(&cm)
		}
	}
}

//export EndDgeo
func EndDgeo() {
	req := peer.RequestMsg{}
	req.RType = peer.REnd
	for p, _ := range aps {
		enc, _ := connect(p)
		if enc != nil {
			enc.Encode(req)
		}
	}
}

//export StoreState
func StoreState(folder *C.char) {
	req := peer.RequestMsg{}
	req.RType = peer.RStoreState
	for p, _ := range aps {
		enc, _ := connect(p)
		if enc != nil {
			enc.Encode(req)
			stm := peer.StoreFolderMsg{}
			stm.Folder = C.GoString(folder)
			enc.Encode(stm)
		}
	}
}

//export LoadState
func LoadState(folder *C.char) {
	req := peer.RequestMsg{}
	req.RType = peer.RLoadState
	for p, _ := range aps {
		enc, _ := connect(p)
		if enc != nil {
			enc.Encode(req)
			stm := peer.StoreFolderMsg{}
			stm.Folder = C.GoString(folder)
			enc.Encode(stm)
		}
	}
}

func GetRemoteDatasetsMetadata() {
	req := peer.RequestMsg{}
	req.RType = peer.RGetDatasetsMeta
	var p string
	for p, _ = range aps {
		break;
	}
	enc, dec := connect(p)
	if enc != nil {
		enc.Encode(req)
		log.Printf("Remote datasets:\n")

		gdm := peer.GetDatasetsMeta{}
		err := dec.Decode(&gdm)
		if err != nil {
			peer.Flog.Fatal(err)
		}

		for _, nds := range gdm.Dts {
			log.Printf("%s\n", nds.Dataset)

			cdsname := C.CString(nds.Dataset)
			dataset := C.dataset_create_mem(cdsname, 1)
			dataset.metadata.servers = C.int(len(aps))
			dataset.metadata.geocount = C.uint(nds.GeoCount)
			C.free(unsafe.Pointer(cdsname))

			hist := unsafe.Pointer(&nds.Metadata[0])
			C.dataset_set_histogram(dataset, (*C.dataset_histogram_persist)(hist))

			C.add_new_dataset(dataset)
		}
	}
}

func main() {
	log.SetOutput(os.Stdout)
	log.Println("DGEO Build/Version: ", peer.APP_VERSION)

	ap = peer.ActivePeers{}
	go ap.Start(udp_group, true, "0")
	time.Sleep(time.Second * 2)
	aps = ap.ActivePeers()

        for p, i := range aps {
			log.Printf("%d: %s\n", i, p)
	}

	if len(aps) > 0 {
		GetRemoteDatasetsMetadata()
	}

	if len(os.Args) > 1 {
		C.startparser(C.CString(os.Args[1]))
	} else {
		C.startparser(nil)
	}
}
