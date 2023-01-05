package main

/*
#include "system.h"
*/
import "C"

import (
	"encoding/gob"
	"log"
	"dgeodistr/peer"
	"unsafe"
)

var (
	open_conns  map[string]chan peer.LoadMsg
	peers       map[byte]string
	dispatchers chan peer.LoadMsg
)

//export NewDataset
func NewDataset(dataset *C.char, histogram unsafe.Pointer, histsize C.int, intermed C.int) {

	godataset := C.GoString(dataset)

	open_conns = make(map[string]chan peer.LoadMsg)
	peers = make(map[byte]string)
	dispatchers = make(chan peer.LoadMsg)

	req := peer.RequestMsg{}
	req.RType = peer.RLoad

	nds := peer.NewDataset{}
	nds.Dataset = godataset
	nds.Metadata = C.GoBytes(histogram, histsize)
	if intermed == 0 {
		nds.Intermed = false
	} else {
		nds.Intermed = true
	}

	for p, v := range aps {
		peers[v] = p

		enc, dec := connect(p)
		if enc != nil {
			chann := make(chan peer.LoadMsg, 5)
			open_conns[p] = chann
			go dispatchWkb(req, nds, enc, dec, chann, godataset)
		}
	}
}

//export EndDataset
func EndDataset() {
	for _, v := range open_conns {
		close(v)
	}
	for _ = range peers {
		<-dispatchers
	}
}

//export SendWkb
func SendWkb(server C.int, x C.ushort, y C.ushort, gid C.longlong, iscopy C.bool, wkb unsafe.Pointer, i C.int) {
	gi := peer.LoadMsg{}
	gi.Sid = peer.SegmentId{uint16(x), uint16(y)}
	gi.Gid = int64(C.longlong(gid))
	gi.IsCopy = bool(iscopy)
	gi.Wkb = C.GoBytes(wkb, i)
	open_conns[peers[byte(server)]] <- gi
	//log.Println(int(server))
}

func dispatchWkb(req peer.RequestMsg, nds peer.NewDataset, enc *gob.Encoder, dec *gob.Decoder, wkbs chan peer.LoadMsg, dataset string) {

	if !checkDispatch(enc.Encode(req)) {
		return
	}

	if !checkDispatch(enc.Encode(nds)) {
		return
	}

	for m := range wkbs {
		if !checkDispatch(enc.Encode(m)) {
			return
		}
	}

	mf := peer.LoadMsg{}
	mf.End = true
	if !checkDispatch(enc.Encode(mf)) {
		return
	}

	lm := peer.LoadMsg{}
	err := dec.Decode(&lm)
	if err != nil {
		log.Fatal(err)
	}
	dispatchers <- lm
}
