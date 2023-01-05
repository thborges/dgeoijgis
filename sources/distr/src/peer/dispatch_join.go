package peer

/*
#cgo CFLAGS: -std=gnu11 -I../../../ -I../../../utils/ -I../../../join/
#include <resource.h>
#include <utils.h>
#include <dataset.h>
#include <histcommon.h>
#include <ss_djoin.h>
#include <ss_dscopy.h>
#include <geos_c.h>
*/
import "C"

import (
	"encoding/gob"
	"fmt"
	"log"
	"net"
	"os"
	"runtime"
	"sync"
	"sync/atomic"
	"time"
	"unsafe"
)

var (
	workers_count = runtime.GOMAXPROCS(0)*2
	disp_workers = make(chan int, workers_count)
)

type JoinMsgCopyWrap struct {
	jmw     *JoinMsgWrap
	copyidx int
}

type CompareResult struct {
	Level          uint8
	Card           uint32
	InstantReturns InstantStepsReturn
}

func (j *JoinMsgWrap) DoneCopying(qtd int) bool {
	j.copymtx.Lock()
	j.copycnt = j.copycnt - qtd
	done := j.copycnt == 0
	j.copymtx.Unlock()
	return done
}

type CompareBlock struct {
	lds      *C.dataset
	lseg     *C.dataset_segment
	cdss     []C.go_dataset_seg
	sidl     SegmentId
	rs       chan *C.GList
	divcount int
	startpr  time.Time
}

func concat(cb *CompareBlock, resultds *dataset)uint32 {
	// concat threads results into dataset segments
	count := uint32(0)
	resultseg, _ := dataset_geodb_get_or_create_seg(resultds, cb.sidl, false)
	for s := 0; s < cb.divcount; s++ {
		rs := <-cb.rs
		count += uint32(C.add_glist_to_seg(resultds.dataset, resultseg.c_seg, rs))
	}
	//fmt.Fprintf(os.Stdout, "%d_%d\t%d\t%d\n", cb.sidl.X, cb.sidl.Y, cb.divcount, count)
	segment_finish(resultseg)
	//log.Printf("%d.%d\t%d\t%d\n", Level, cb.lseg.sid, time.Since(cb.startpr).Nanoseconds()/1000, count)
	return count
}

func process_cb_slice(cb *CompareBlock, start C.uint, end C.uint, busy chan int,
	Level uint8, jsm JoinStartMsg, geoscontext C.GEOSContextHandle_t, resultds *dataset,
	tcount *uint32) {

	c_cdss := (*C.go_dataset_seg)(unsafe.Pointer(&cb.cdss[0]))
	rs := C.djoin_compare_segs(cb.lds, cb.lseg, start, end,
		c_cdss, C.int(len(cb.cdss)), C.bool(Level+1 == jsm.Levels), geoscontext)
	cb.rs <- rs

	if (start == 0) {
		// The first thread concat the result. This is better 
		// than a separately go routine to concat due to 
		// skewness in each partition
		count := concat(cb, resultds)
		atomic.AddUint32(tcount, count)

		// logic to report/inspect non-blocking behaviour
		//isr.m.Lock()
		//isr.Instants = append(isr.Instants, InstantReturn{time.Now().Local(), count})
		//isr.m.Unlock()
	}
	<-disp_workers // one worker available
	<-busy
}

func GoJoinCompare(tocompare chan *JoinMsg, Level uint8, jsm JoinStartMsg, endsignal chan CompareResult,
	signaljob chan bool) {

	geoscontext := C.init_geos_context()

	var isr InstantStepsReturn
	var resultds *dataset
	tcount := uint32(0)
	busy := make(chan int, workers_count)

	// process the predicate
	process_cb := func(cb *CompareBlock) {
		cb.startpr = time.Now()

		if (cb.lseg.count == 0) {
			cb.divcount = 0
			concat(cb, resultds)
			return
		}

		qtd := cb.lseg.count / C.uint(workers_count) +1
		cb.divcount = 0
		for current := C.uint(0); current < cb.lseg.count; current += qtd {
			cb.divcount += 1
		}
		cb.rs = make(chan *C.GList, cb.divcount)

		for current := C.uint(0); current < cb.lseg.count; current += qtd {
			disp_workers <- 0 // one worker busy
			busy <- 0
			go process_cb_slice(cb, current, current+qtd, busy, Level, jsm, geoscontext, resultds, &tcount)
		}
	}

	process_jm := func(jm *JoinMsg) {
		cb := CompareBlock{}
		leftSeg := jm.L

		// wait for dataset
		d, s := dataset_get_wait_complete(leftSeg.Dataset, leftSeg.Sid)
		cb.lds, cb.lseg = d.dataset, s.c_seg

		i := 0
		var cdss = make([]C.go_dataset_seg, len(jm.R))
		for _, rightSeg := range jm.R {
			rds, rseg := dataset_get_wait_complete(rightSeg.Dataset, rightSeg.Sid)
			cdss[i].ds = rds.dataset
			cdss[i].sseg = rseg.c_seg
			i += 1
		}
		cb.cdss = cdss
		cb.sidl = leftSeg.Sid

		process_cb(&cb)
	}

	result_name := fmt.Sprintf("%s_step%d", jsm.QueryName, Level)
	resultds, _ = dataset_geodb_get_or_create_ds(result_name)
	resultds.Intermed = true

	// Get the work and wait for the datasets
	for _ = range signaljob {
		jm := <-tocompare
		process_jm(jm)
	}

	// wait local jobs to finish
	for w := 0; w < workers_count; w++ {
		busy <- 0
	}

	count := atomic.LoadUint32(&tcount)
	endsignal <- CompareResult{Level, count, isr}
	C.finishGEOS_r(geoscontext)
}

func CopyDatasetSeg(dsc DatasetSeg, enc *gob.Encoder, dec *gob.Decoder) {

	ds, ds_seg, copy_needed := dataset_get_seg_for_copying(dsc.Dataset, dsc.Sid)
	if !copy_needed {
		//log.Printf("Dataset alread exists here: %s %d %d\n", dsc.Dataset, dsc.Sid.X, dsc.Sid.Y)
		return
	}

	// log.Printf("Copying dataset %s, %d_%d\n", dsc.Dataset, dsc.Sid.X, dsc.Sid.Y)

	// request copy
	cm := CopyMsg{}
	cm.Dataset = dsc.Dataset
	cm.Sid = dsc.Sid
	err := enc.Encode(cm)
	if err != nil {
		Flog.Fatal(err)
	}

	// dataset head
	nds := NewDataset{}
	err = dec.Decode(&nds)
	if err != nil {
		Flog.Fatal(err)
	}

	//dataset_set_histogram(ds, nds.Metadata)
	ds.Intermed = nds.Intermed
	ds_seg.Replica = true

	addchan := make(chan *LoadMsg, 2)

	go func() {
		for {
			m := LoadMsg{}
			err := dec.Decode(&m)
			if err != nil {
				Flog.Fatal(err)
			}
			if m.End {
				break
			}
			addchan <- &m
		}
		close(addchan)
	}()

	for m := range addchan {
		wkb := m.Wkb[0:]
		c_wkb := (*C.uchar)(unsafe.Pointer(&wkb[0]))

		var mbr C.Envelope
		mbr = C.GetEnvelope(C.double(m.Mbr.MinX), C.double(m.Mbr.MinY), C.double(m.Mbr.MaxX), C.double(m.Mbr.MaxY))

		C.add_geoitem(ds.dataset, ds_seg.c_seg, C.longlong(m.Gid), C.bool(m.IsCopy), c_wkb, C.uint(len(wkb)), &mbr)
	}

	segment_finish(ds_seg)

	// log.Printf("Copyed dataset %s, %d_%d\n", dsc.Dataset, dsc.Sid.X, dsc.Sid.Y)
}

func GoCopyFromServer(server string, s ServerChan) {

	var conn net.Conn;
	var err error;
	for {
		conn, err = net.Dial("tcp", server)
		if err == nil {
			break;
		} else {
			log.Printf("Error connecting to %s. %s. Retrying...\n", server, err)
		}
	}

	dec := gob.NewDecoder(conn)
	enc := gob.NewEncoder(conn)

	req := RequestMsg{}
	req.RType = RCopy
	err = enc.Encode(req)
	if err != nil {
		Flog.Fatal(err)
	}

	for p := range s.copyChan {
		if p.copyidx == -1 {
			CopyDatasetSeg(p.jmw.jm.L, enc, dec)
		} else {
			CopyDatasetSeg(p.jmw.jm.R[p.copyidx], enc, dec)
		}
		if p.jmw.DoneCopying(1) {
			s.tocompare[p.jmw.jm.Level] <- p.jmw.jm
		}
	}

	conn.Close()
	s.done <- true
}

func CreateServerChan(host string, tocompare []chan *JoinMsg) ServerChan {
	s := ServerChan{make(chan JoinMsgCopyWrap, 50), tocompare, make(chan bool)}
	go GoCopyFromServer(host, s)
	return s
}

func GoJoinCopyRequest(copiers map[string]ServerChan, tocopy chan *JoinMsgWrap,
	tocompare []chan *JoinMsg, endsignal chan bool, hp *GeoServiceHandlerParams) {

	peers := hp.ap.ActivePeers()
	for host, _ := range peers {
		if host != hp.addr {
			log.Printf("Creating copy channel to %s\n", host)
			s := CreateServerChan(host, tocompare)
			copiers[host] = s
		}
	}
	if len(copiers) == len(peers) {
		// prevent that join routine copy segments from the local host
		log.Printf("\n\n-----\n\033[91mCurrent server %s doesn't exists in peers addrs list.\nIs your hosts file correct? \033[0m\n-----\n\n", hp.addr)
		os.Exit(1)
	}

	for jmw := range tocopy {
		//log.Println("GoJoinCopy:", jmw.jm.L.Host, jmw.jm.L.Sid.X, jmw.jm.L.Sid.Y)

		if !jmw.jm.L.copied {
			copiers[jmw.jm.L.Host].copyChan <- JoinMsgCopyWrap{jmw, -1}
		}
		for idx, rds := range jmw.jm.R {
			if !rds.copied {
				copiers[rds.Host].copyChan <- JoinMsgCopyWrap{jmw, idx}
			}
		}
	}

	for _, s := range copiers {
		close(s.copyChan)
	}

	for _, s := range copiers {
		<-s.done
	}

	endsignal <- true
}

func JoinStatsCapture(hp *GeoServiceHandlerParams, em chan JoinEndMsg, done chan bool,
	copiers map[string]ServerChan, tocompare []chan *JoinMsg) {

	intf := hp.flags.Lookup("intf").Value.String()
	log.Println("Interface measured:", intf)

	// first usage immediatelly
	jem := JoinEndMsg{}
	jem.RegisterUsage(intf, copiers, tocompare[0:])

	// last usage
	defer func() {
		jem.RegisterUsage(intf, copiers, tocompare[0:])
		em <- jem
	}()

	// register usage on a basis (a quarter of second)
	//timer := time.NewTicker(time.Millisecond*250).C
	timer := time.NewTicker(time.Second).C
	for {
		select {
		case <-timer:
			jem.RegisterUsage(intf, copiers, tocompare[0:])
		case <-done:
			return
		}
	}
}

func Join(hp *GeoServiceHandlerParams) {
	jsm := JoinStartMsg{}
	hp.dec.Decode(&jsm)
	log.Println("Join request with", jsm.Levels, "levels")

	copiers := make(map[string]ServerChan)
	tocopy := make(chan *JoinMsgWrap, 50)
	tocompare := make([]chan *JoinMsg, jsm.Levels)
	signaljob := make([]chan bool, jsm.Levels)
	for i := uint8(0); i < jsm.Levels; i++ {
		tocompare[i] = make(chan *JoinMsg, 100)
		signaljob[i] = make(chan bool, 100)
	}

	endStats := make(chan JoinEndMsg)
	doneStats := make(chan bool)
	go JoinStatsCapture(hp, endStats, doneStats, copiers, tocompare[0:])

	endsignal := make(chan CompareResult)
	endcopy := make(chan bool)

	go GoJoinCopyRequest(copiers, tocopy, tocompare, endcopy, hp)

	// compare results
	rcl := make([]CompareResult, jsm.Levels)

	// the first two starts now
	// the others start when the first two end
	done_launching_compare := make(chan bool)
	start_compare_for_levels := func() {
		go GoJoinCompare(tocompare[0], 0, jsm, endsignal, signaljob[0])
		/*if jsm.Levels > 1 {
			go GoJoinCompare(tocompare[1], 1, jsm, endsignal, signaljob[1])
		}*/
		for i := uint8(1); i < jsm.Levels; i++ {
			// wait the first levels end
			cr := <-endsignal
			rcl[cr.Level] = cr
			log.Println("Level finished:", cr.Level)
			go GoJoinCompare(tocompare[i], i, jsm, endsignal, signaljob[i])
		}
		done_launching_compare <- true
	}
	go start_compare_for_levels()

	process_chan := make(chan *JoinMsg, 2)

	jmsgs := 0
	go func() { //receive join messages
		for {
			jm := JoinMsg{}
			err := hp.dec.Decode(&jm)
			if err != nil {
				log.Println("Unexpected end of the protocol: ", err)
				break
			}

			if jm.End {
				break
			}

			jmsgs++
			process_chan <- &jm
		}
		close(process_chan)
	}()

	// assumes that jobs are send ordered by level
	curr_level := uint8(255)
	for jm := range process_chan {
		copies := 0
		jm.L.copied = jm.L.Host == hp.addr
		all_this_host := jm.L.copied
		if !jm.L.copied {
			copies = copies + 1
		}

		for i := 0; i < len(jm.R); i++ {
			jm.R[i].copied = jm.R[i].Host == hp.addr
			all_this_host = all_this_host && jm.R[i].copied
			if !jm.R[i].copied {
				copies = copies + 1
			}
		}

		if all_this_host { // this host
			tocompare[jm.Level] <- jm
		} else {
			tocopy <- &JoinMsgWrap{jm, copies, &sync.Mutex{}}
		}

		if curr_level != jm.Level {
			if curr_level != uint8(255) {
				close(signaljob[curr_level])
			}
			curr_level = jm.Level
		}

		signaljob[curr_level] <- true
	}
	close(signaljob[curr_level])

	//log.Printf("Ending copy and compare (%d) go routines\n", jmsgs)
	close(tocopy)
	<-endcopy //copy
	<-done_launching_compare

	// two levels remained from above
	//for i := uint8(0); i < 2; i++ {
		cr := <-endsignal
		rcl[cr.Level] = cr
		log.Println("Level finished:", cr.Level)
	//}

	doneStats <- true
	jem := <-endStats
	jem.ResultCardinality = make([]uint32, jsm.Levels)
	jem.StepsReturns = make([]InstantStepsReturn, jsm.Levels)
	for i := uint8(0); i < jsm.Levels; i++ {
		jem.ResultCardinality[i] = rcl[i].Card
		jem.StepsReturns[i] = rcl[i].InstantReturns
	}

	log.Println("Sending result message with size", jem.ResultCardinality)
	hp.enc.Encode(jem)
}
