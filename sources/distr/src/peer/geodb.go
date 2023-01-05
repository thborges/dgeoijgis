package peer

/*
#cgo pkg-config: gdal
#cgo CFLAGS: -I. -I${SRCDIR}/../../../utils/ -I${SRCDIR}/../../../stats/
#cgo LDFLAGS: -L${SRCDIR}/../../../utils -L${SRCDIR}/../../../stats/
#cgo LDFLAGS: -lrtreeutils -ldgeo_stats -lifstat -lgeos -lgeos_c -lm -lstdc++
#include <stdlib.h>
#include <geos_c.h>
#include <dataset.h>
#include <histcommon.h>
#include <utils.h>
*/
import "C"

import (
	"encoding/gob"
	"log"
	"os"
	"sync"
	"unsafe"
	"path/filepath"
)

var (
	datasets        = map[string]*dataset{}
	datasets_rwlock = sync.RWMutex{}
	db_file = "datasets.gob"
)

type dataset struct {
	dataset         *C.dataset
	segments_rwlock sync.RWMutex
	Segments        map[SegmentId]*dataset_segment
	Intermed        bool
	Name			string
}

type dataset_segment struct {
	Sid      SegmentId
	cond     *sync.Cond
	Complete bool
	Copying  bool
	Replica  bool
	c_seg    *C.dataset_segment
}

func dataset_get_wait_complete(dataset_name string, sid SegmentId) (*dataset, *dataset_segment) {

	ds, _ := dataset_geodb_get_or_create_ds(dataset_name)
	seg, _ := dataset_geodb_get_or_create_seg(ds, sid, false)

	wait := false
	seg.cond.L.Lock()
	for !seg.Complete {
		log.Printf("Waiting for completion of %s_%d_%d\n", dataset_name, sid.X, sid.Y)
		wait = true
		seg.cond.Wait()
	}
	seg.cond.L.Unlock()

	if wait {
		log.Printf("Waiting for completion of %s_%d_%d: ok\n", dataset_name, sid.X, sid.Y)
	}

	return ds, seg
}

func dataset_get_all_segs(dataset_name string) (*dataset, map[SegmentId]*dataset_segment) {
	datasets_rwlock.RLock()
	defer datasets_rwlock.RUnlock()

	ds, ok := datasets[dataset_name]
	if !ok {
		return nil, nil
	} else {
		return ds, ds.Segments
	}
}

func dataset_geodb_get_or_create_seg(ds *dataset, sid SegmentId, for_copying bool) (*dataset_segment, bool) {

	copy_needed := false
	ds.segments_rwlock.Lock()
	seg, ok := ds.Segments[sid]
	if !ok {
		newmutex := sync.Mutex{}
		seg = &dataset_segment{}
		seg.cond = sync.NewCond(&newmutex)
		seg.Complete = false
		seg.Copying = false
		seg.Sid = sid
		seg.c_seg = C.dataset_new_seg(ds.dataset, C.uint(sid.GetCSid()))

		ds.Segments[sid] = seg
	}
	if for_copying && seg.Complete == false && !seg.Copying {
		seg.Copying = true
		copy_needed = true
	}
	ds.segments_rwlock.Unlock()
	return seg, copy_needed
}

func dataset_geodb_get_or_create_ds(dataset_name string) (*dataset, bool) {

	created := false
	datasets_rwlock.RLock()
	ds, ok := datasets[dataset_name]
	datasets_rwlock.RUnlock()

	if !ok {
		datasets_rwlock.Lock()
		// check again after wlock to prevent race
		ds, ok = datasets[dataset_name]
		if !ok {
			ds = &dataset{}
			ds.Name = dataset_name
			ds.dataset = nil
			ds.segments_rwlock = sync.RWMutex{}
			ds.Segments = make(map[SegmentId]*dataset_segment)

			cdsname := C.CString(dataset_name)
			c_ds := C.dataset_create(cdsname, 1)
			C.free(unsafe.Pointer(cdsname))
			ds.dataset = c_ds

			datasets[dataset_name] = ds
			created = true
		}
		datasets_rwlock.Unlock()
	}
	return ds, created
}

func dataset_get_seg_for_copying(dataset_name string, sid SegmentId) (*dataset, *dataset_segment, bool) {

	ds, _ := dataset_geodb_get_or_create_ds(dataset_name)
	seg, copy_needed := dataset_geodb_get_or_create_seg(ds, sid, true)
	return  ds, seg, copy_needed
}

func dataset_set_histogram(ds *dataset, histogram []byte) {

	datasets_rwlock.Lock()
	if histogram == nil {
		// prevent creation of dataset without a histogram 
		// (when copying from another host, the dataset need to be already created locally by the client)
		log.Printf("\n\n-----\n\033[91mCan't set dataset %s histogram = nil.\033[0m\n-----\n\n", ds.Name)
		os.Exit(1)
	}
	C.dataset_set_histogram(ds.dataset, (*C.dataset_histogram_persist)(unsafe.Pointer(&histogram[0])))
	datasets_rwlock.Unlock()
}

func segment_finish(seg *dataset_segment) {

	seg.cond.L.Lock()
	if seg.Complete {
		Flog.Fatal("Segment alread completed: ", seg)
	}
	seg.Complete = true
	seg.Copying = false

	seg.cond.Broadcast()
	seg.cond.L.Unlock()
}

func geodb_clean() {
    // first clean intermediate results to prevent 
    // freeing the geos two times
    geodb_clean_intermediate()

	datasets_rwlock.Lock()
	for _, ds := range datasets {
		C.dataset_destroy(ds.dataset)
	}
	datasets = map[string]*dataset{}
	datasets_rwlock.Unlock()
}

func geodb_clean_intermediate() {
	datasets_rwlock.Lock()
	newdatasets := map[string]*dataset{}
	for name, ds := range datasets {
		if ds.Intermed {
			// destroy intermediate datasets
			C.dataset_destroy_full(ds.dataset, 0)
		} else {
			newdatasets[name] = ds
			old_segments := ds.Segments

			// destroy segments replicated in a prior join
			ds.Segments = map[SegmentId]*dataset_segment{}
			for sid, seg := range old_segments {
				if seg.Replica {
					C.dataset_destroy_seg(ds.dataset, seg.c_seg)
				} else {
					ds.Segments[sid] = seg
				}
			}
		}
	}
	datasets = newdatasets
	datasets_rwlock.Unlock()
}

func geodb_store_to_disk(folder string) (error) {
	log.Printf("Storing datasets in %s:\n", folder)

	geodb_clean_intermediate()

	os.Mkdir(folder, os.ModePerm)
	file, err := os.Create(filepath.Join(folder, db_file))
	if err == nil {
		encoder := gob.NewEncoder(file)
		err = encoder.Encode(datasets)
		if err != nil {
			Flog.Fatal(err)
		}
		file.Close()

		for name, ds := range datasets {
			log.Printf("\t%s\n", name)
			fname := C.CString(filepath.Join(".", folder, name) + ".cdat")
			C.dataset_store_to_disk(ds.dataset, fname)
			C.free(unsafe.Pointer(fname))
		}
	} else {
		log.Print(err)
		return err
	}

	log.Printf("Done storing datasets.\n")
	return nil
}

func geodb_load_from_disk(folder string) {
	log.Printf("Loading stored datasets in %s:\n", folder)

	file, err := os.Open(filepath.Join(folder, db_file))
	if err == nil {
		decoder := gob.NewDecoder(file)
		err = decoder.Decode(&datasets)
		if err != nil {
			Flog.Fatal(err)
		}
		file.Close()

		// restore transient fields
		for name, ds := range datasets {
			log.Printf("\t%s\n", name)
			dname := C.CString(name)
			fname := C.CString(filepath.Join(".", folder, name) + ".cdat")
			ds.dataset = C.dataset_load_from_disk(dname, fname)
			C.free(unsafe.Pointer(fname))
			C.free(unsafe.Pointer(dname))
			ds.segments_rwlock = sync.RWMutex{}

			for _, seg := range ds.Segments {
				newmutex := sync.Mutex{}
				seg.cond = sync.NewCond(&newmutex)
				seg.c_seg = C.dataset_get_seg(ds.dataset, C.uint(seg.Sid.GetCSid()))

				//log.Printf("Segment %s:%d.%d", name, seg.Sid.X, seg.Sid.Y)
				if !seg.Complete {
					Flog.Fatal("Segment not completed!")
				}
				if seg.Replica {
					Flog.Fatal("Segment is a copy!")
				}
			}
		}
	}
	log.Printf("Done loading stored datasets.\n")
}

func init() {
	C.init_geos()
}
