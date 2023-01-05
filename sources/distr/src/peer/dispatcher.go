package peer

/*
#include <dataset.h>
#include <histcommon.h>
#include <ss_dscopy.h>
#cgo LDFLAGS: -lm
*/
import "C"

import (
	"log"
	"runtime"
	"unsafe"
)

type RequestType int
type Dispatcher map[RequestType]func(*GeoServiceHandlerParams)

const (
	RLoad RequestType = iota
	RJoin
	RCopy
	RClean
	RCleanIntermediate
	REnd
	RGetDatasets
	RGetDatasetItems
	RGetDatasetsMeta
	RStoreState
	RLoadState
)

func Load(hp *GeoServiceHandlerParams) {

	newdataset := NewDataset{}
	err := hp.dec.Decode(&newdataset)
	if err != nil {
		Flog.Fatal(err)
	}

	log.Println("New dataset:", newdataset.Dataset)
	ds, _ := dataset_geodb_get_or_create_ds(newdataset.Dataset)
	ds.Intermed = newdataset.Intermed
	dataset_set_histogram(ds, newdataset.Metadata)

	segments_seen := make(map[uint32]*dataset_segment)

	maxseg := ^uint16(0)

	i := 0
	iall := 0
	current_seg_id := SegmentId{maxseg, maxseg}
	var current_seg *dataset_segment

	for {
		m := LoadMsg{}
		err := hp.dec.Decode(&m)
		if err != nil {
			log.Println("Unexpected end of the protocol: ", err)
			break
		}

		if m.End {
			log.Printf("Load ended with %d items, %d with clones.\n", i, iall)
			break
		}

		if current_seg_id.IsDiff(m.Sid) {
			current_seg_id = m.Sid

			_, ok := segments_seen[current_seg_id.GetCSid()]
			if !ok {
				current_seg, _ = dataset_geodb_get_or_create_seg(ds, current_seg_id, false)
				segments_seen[current_seg_id.GetCSid()] = current_seg
				//log.Printf("Segment start %d %d.\n", current_seg.sid.X, current_seg.sid.Y)
			} else {
				current_seg = segments_seen[current_seg_id.GetCSid()]
			}
		}

		wkb := m.Wkb[0:]
		c_wkb := (*C.uchar)(unsafe.Pointer(&wkb[0]))

		C.add_geoitem(ds.dataset, current_seg.c_seg, C.longlong(m.Gid), C.bool(m.IsCopy), c_wkb, C.uint(len(wkb)), nil)

		if !m.IsCopy {
			i++
		}
		iall++
	}

	for _, s := range segments_seen {
		segment_finish(s)
	}

	lm := LoadMsg{}
	lm.End = true
	hp.enc.Encode(lm)
}

func Copy(hp *GeoServiceHandlerParams) {
	for {
		m := CopyMsg{}
		err := hp.dec.Decode(&m)
		if err != nil {
			log.Printf("Ending copy request: %s", err)
			break
		}
		//log.Printf("Requested copy of dataset %s, %d_%d\n", m.Dataset, m.Sid.X, m.Sid.Y)

		ds, seg := dataset_get_wait_complete(m.Dataset, m.Sid)
		nds := NewDataset{}
		nds.Dataset = m.Dataset
		nds.Intermed = ds.Intermed

		/*var size C.int
		hdata := unsafe.Pointer(C.dataset_get_histogram(ds.dataset, &size))
		nds.Metadata = C.GoBytes(hdata, size)
		C.g_free(hdata)*/

		err = hp.enc.Encode(nds)
		if err != nil {
			log.Printf("Error sending dataset head %s, %d_%d\n", m.Dataset, m.Sid.X, m.Sid.Y)
			return
		}

		errorcopy := false
		sendchan := make(chan LoadMsg, 50)

		go func() {
			cc := C.start_copy_context()
			for it := C.dataset_first_seg(ds.dataset, seg.c_seg); it.item != nil; C.dataset_next_seg(&it) {
				if errorcopy {
					return
				}

				var wkb_size C.int
				var is_copy C.bool
				var gid C.longlong
				var minx, miny, maxx, maxy C.double
				wkb := C.get_wkb(cc, it.item, &wkb_size, &gid, &is_copy)

				m := LoadMsg{}
				m.IsCopy = bool(is_copy)
				m.Gid = int64(gid)
				m.Wkb = C.GoBytes(unsafe.Pointer(wkb), wkb_size)

				C.GetEnvelopeCoordinates(it.item.mbr, &minx, &miny, &maxx, &maxy)
				m.Mbr = Envelope{float64(minx), float64(miny), float64(maxx), float64(maxy)}

				sendchan <- m
			}
			C.end_copy_context(cc)

			close(sendchan)
		}()

		for m := range sendchan {
			err = hp.enc.Encode(m)
			if err != nil {
				log.Printf("Error sending item wkb: %s", err)
				errorcopy = true
				break
			}
		}

		me := LoadMsg{}
		me.End = true
		err = hp.enc.Encode(me)
		if err != nil {
			log.Printf("Error sending dataset end: %s", err)
			return
		}
	}
}

func CopyDataset(hp *GeoServiceHandlerParams) {
	for {
		query := CopyMsg{}
		err := hp.dec.Decode(&query)
		if err != nil {
			log.Printf("Ending copy request: %s", err)
			break
		}
		log.Printf("Requested copy of dataset %s\n", query.Dataset)

		dsname := query.Dataset
		histogram := false
		if query.Dataset[:7] == "_hist__" {
			histogram = true
			dsname = query.Dataset[7:]
			log.Printf("Requested histogram copy for ds %s\n", dsname);
		}

		ds, segs := dataset_get_all_segs(dsname)
		if ds == nil {
			break;
		}
		nds := NewDataset{}
		nds.Dataset = query.Dataset
		nds.GeoCount = int(ds.dataset.metadata.geocount)
		err = hp.enc.Encode(nds)
		if err != nil {
			log.Printf("Error sending dataset head %s\n", query.Dataset)
			return
		}

		errorcopy := false
		sendchan := make(chan CopyItemMsg, 50)

		go func() {
			cc := C.start_copy_context_wkt()
			fenv := C.GetEnvelope(C.double(query.BBox.MinX), C.double(query.BBox.MinY),
				C.double(query.BBox.MaxX), C.double(query.BBox.MaxY))
			for _, seg := range segs {
				if seg.Replica {
					continue
				}

				// report histogram cells
				if histogram {
					m := CopyItemMsg{}
					m.WKT = make([]string, nds.GeoCount)
					m.Gid = int64(int(seg.Sid.X) * 10000 + int(seg.Sid.Y));
					m.Card = int(seg.c_seg.count);
					m.WKT[0] = C.GoString(C.get_histo_cell_wkt(ds.dataset,
						C.ushort(seg.Sid.X), C.ushort(seg.Sid.Y)));
					sendchan <- m

				} else {
					for it := C.dataset_first_seg(ds.dataset, seg.c_seg); it.item != nil; C.dataset_next_seg(&it) {
						if errorcopy {
							return
						}

						if query.Filter && bool(C.filtered(fenv, C.int(nds.GeoCount), it.item)) {
							continue
						}

						var is_copy C.bool
						var gid C.longlong
						m := CopyItemMsg{}
						m.WKT = make([]string, nds.GeoCount)
						for i := C.int(0); i < C.int(nds.GeoCount); i++ {
							wkt := C.get_wkt(cc, it.item, C.int(i), &gid, &is_copy)
							if !ds.Intermed && bool(is_copy) {
								break;
							} else {
								m.Gid = int64(gid)
								m.WKT[i] = C.GoString(wkt)
							}
						}
						if ds.Intermed || !bool(is_copy) {
							sendchan <- m
						}
					}
				}
			}
			C.end_copy_context_wkt(cc)

			close(sendchan)
		}()

		for m := range sendchan {
			err = hp.enc.Encode(m)
			if err != nil {
				log.Printf("Error sending item wkt: %s", err)
				errorcopy = true
				break
			}
		}

		me := LoadMsg{}
		me.End = true
		err = hp.enc.Encode(me)
		if err != nil {
			log.Printf("Error sending dataset end: %s", err)
			return
		}
	}
}

func StoreState(hp *GeoServiceHandlerParams) {
	fldr := StoreFolderMsg{}
	err := hp.dec.Decode(&fldr)
	if err != nil {
		log.Printf("Error waiting for folder name: %s", err)
		return
	}
	geodb_store_to_disk(fldr.Folder)
}

func LoadState(hp *GeoServiceHandlerParams) {
	geodb_clean()
	fldr := StoreFolderMsg{}
	err := hp.dec.Decode(&fldr)
	if err != nil {
		log.Printf("Error waiting for folder name: %s", err)
		return
	}
	geodb_load_from_disk(fldr.Folder)
}

func End(hp *GeoServiceHandlerParams) {
	Flog.Fatal("End request. Closing")
}

func Clean(hp *GeoServiceHandlerParams) {
	log.Println("Cleaning geo db")
	geodb_clean()
	runtime.GC()
	cm := CleanMsg{}
	hp.enc.Encode(cm)
}

func CleanIntermediate(hp *GeoServiceHandlerParams) {
	log.Println("Cleaning intermediate datasets")
	geodb_clean_intermediate()
	runtime.GC()
	cm := CleanIntermediateMsg{}
	hp.enc.Encode(cm)
}

func GetDatasets(hp *GeoServiceHandlerParams) {
	dss := GetDatasetsMsg{}
	dss.Dts = make([]DatasetMetadata, len(datasets)*2)
	i := 0
	for name, ds := range datasets {
		dss.Dts[i].Name = name
		dss.Dts[i].GeoCount = int(ds.dataset.metadata.geocount)

		var minx, miny, maxx, maxy C.double
		C.GetEnvelopeCoordinates(ds.dataset.metadata.mbr, &minx, &miny, &maxx, &maxy)
		dss.Dts[i].BBox = Envelope{float64(minx), float64(miny), float64(maxx), float64(maxy)}
		i++

		// histogram
		dss.Dts[i].Name = "_hist__" + name
		dss.Dts[i].GeoCount = 1
		dss.Dts[i].BBox = dss.Dts[i-1].BBox
		i++
	}
	hp.enc.Encode(dss)
}

func GetDatasetsMetadata(hp *GeoServiceHandlerParams) {

	i := 0
	dss := GetDatasetsMeta{}
	for name, ds := range datasets {
		if ds.Intermed {
			continue
		}

		nds := NewDataset{}
		nds.Dataset = name
		nds.Intermed = ds.Intermed

		var size C.int
		hdata := unsafe.Pointer(C.dataset_get_histogram(ds.dataset, &size))
		nds.Metadata = C.GoBytes(hdata, size)
		C.g_free(hdata)
		dss.Dts = append(dss.Dts, nds)

		i++
	}
	err := hp.enc.Encode(dss)
	if err != nil {
		log.Printf("Error sending datasets metadata\n")
		return
	}
}

func GetDispatcher() Dispatcher {
	result := make(Dispatcher)

	result[RLoad] = Load
	result[RJoin] = Join
	result[RCopy] = Copy
	result[RClean] = Clean
	result[RCleanIntermediate] = CleanIntermediate
	result[REnd] = End
	result[RGetDatasets] = GetDatasets
	result[RGetDatasetItems] = CopyDataset
	result[RGetDatasetsMeta] = GetDatasetsMetadata
	result[RStoreState] = StoreState
	result[RLoadState] = LoadState

	return result
}
