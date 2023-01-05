/*
 * dataset.c
 *
 *  Created on: 26/07/2014
 *      Author: 
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include "dataset.h"
#include "histcommon.h"
#include "uthash.h"
#include "geosext.h"

void dataset_store_to_disk(dataset *ds, char *filename) {
	
	// filename
	int dsfile = open(filename, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	
	// clear pointers
	dataset auxds = *ds;
	auxds.temp_ogr_layer = NULL;
	auxds.fpages = 0;
	auxds.segments = NULL;
	auxds.mem_get_lock = NULL;

	// dataset head
	int aux = write(dsfile, &auxds, sizeof(dataset));
	assert(aux == sizeof(dataset));

	// histogram
	int hsize = 0;
	dataset_histogram_persist *hp = ds->metadata.hist.get_data(ds, &hsize);
	aux = write(dsfile, &hsize, sizeof(int));
	assert(aux == sizeof(int));
	aux = write(dsfile, hp, hsize);
	assert(aux == hsize);

    GEOSContextHandle_t geoscontext = initGEOS_r(geos_messages, geos_messages);
    GEOSWKBWriter *writer = GEOSCreateWKBWritter(geoscontext);
    void *iobuff = GEOSCreateWKBBuffer();

	// segments
	dataset_segment *seg, *seg_tmp;
	HASH_ITER(hh, ds->segments, seg, seg_tmp) {

		aux = write(dsfile, &seg->sid, sizeof(unsigned));
		assert(aux == sizeof(unsigned));
		aux = write(dsfile, &seg->count, sizeof(unsigned));
		assert(aux == sizeof(unsigned));
		
        dataset_iter di;
       	dataset_foreach_seg(di, ds, seg) {
            for (unsigned i = 0; i < ds->metadata.geocount; i++) {

				int wkb_size;
				const char *wkb = GEOSGeomToWKB(writer, iobuff, di.item[i].geo, &wkb_size);

				int aux = write(dsfile, &wkb_size, sizeof(int));
				assert(aux == sizeof(int));
				aux = write(dsfile, &di.item[i].gid, sizeof(long long));
				assert(aux == sizeof(long long));
				aux = write(dsfile, &di.item[i].cloned, sizeof(bool));
				assert(aux == sizeof(bool));
				aux = write(dsfile, wkb, wkb_size);
				assert(aux == wkb_size);
            }
   	    }
	}

	close(dsfile);

    GEOSDestroyWKBWritter(writer, iobuff);
    finishGEOS_r(geoscontext);
}

dataset *dataset_load_from_disk(char *name, char *filename) {

	// filename
	int dsfile = open(filename, O_RDONLY);
	if (dsfile == -1) {
		perror("Error loading dataset file");
		return NULL;
	}

	// clear pointers
	dataset auxds;
	int aux = read(dsfile, &auxds, sizeof(dataset));
	assert(aux == sizeof(dataset));

	dataset *result;
	if (auxds.metadata.memory_dataset)
		result = dataset_create_mem(name, auxds.metadata.geocount);
	else
		result = dataset_create(name, auxds.metadata.geocount);

	// histogram
	int hsize = 0;
	aux = read(dsfile, &hsize, sizeof(int));
	assert(aux == sizeof(int));

	//char histp[hsize];
	char *histp = g_new(char, hsize);
	aux = read(dsfile, histp, hsize);
	assert(aux == hsize);
	
	dataset_histogram_persist *hp = (dataset_histogram_persist*)histp;
	dataset_set_histogram(result, hp);
	g_free(histp);

	// segments
	unsigned sid, count;
	while (1) {
		aux = read(dsfile, &sid, sizeof(unsigned));
		if (aux != sizeof(unsigned)) //end of segments
			break;

		aux = read(dsfile, &count, sizeof(unsigned));
		assert(aux == sizeof(unsigned));

		//printf("Segment %d: %d\n", sid, count);

		dataset_segment *seg = dataset_new_seg(result, sid);
		for(unsigned i = 0; i < count; i++) {
			dataset_leaf *leaf = dataset_add_seg(result, seg);
		
            for (unsigned g = 0; g < result->metadata.geocount; g++) {
				int wkb_size;
				aux = read(dsfile, &wkb_size, sizeof(int));
				assert(aux == sizeof(int));

				long long gid;
				aux = read(dsfile, &gid, sizeof(long long));
				assert(aux == sizeof(long long));

				bool cloned;
				aux = read(dsfile, &cloned, sizeof(bool));
				assert(aux == sizeof(bool));

				unsigned char wkb[wkb_size];
				aux = read(dsfile, wkb, wkb_size);
				assert(aux == wkb_size);

				GEOSGeometryH ggeo = GEOSGeomFromWKB_buf(wkb, wkb_size);
				if (ggeo == NULL) {
					printf("Error converting WKB to GEOS: %s %d wkb %d", result->metadata.name, i, wkb_size);
				}
				dataset_fill_leaf_ext(leaf, 0, gid, cloned, ggeo, NULL, NULL);
            }
   	    }
	}

	close(dsfile);

	return result;
}

