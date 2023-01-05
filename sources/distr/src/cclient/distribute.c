#include <geosext.h>
#include <dataset.h>
#include <utils.h>

extern void SendWkb(int s, unsigned short x, unsigned short y, long long gid, bool iscopy, const char*, int n);

void distribute_data_through_servers(dataset *ds) {

    printf("Distributing cells to servers\n");
    //dataset_histogram *dh = &ds->metadata.hist;

    GEOSContextHandle_t geoscontext = initGEOS_r(geos_messages, geos_messages);
    GEOSWKBWriter *writer = GEOSCreateWKBWritter(geoscontext);
    void *iobuff = GEOSCreateWKBBuffer();

    unsigned read = 0;
    dataset_iter di;
    dataset_foreach(di, ds) {
        dataset_leaf *l = &di.item[0];

        GEOSGeometryH geo = dataset_get_leaf_geo(ds, l);
        if (geo == NULL) {
            fprintf(stderr, "Ignoring object gid:%lld because geo == NULL\n", l->gid);
            continue;
        }

        int wkb_size;
        const char *wkb = GEOSGeomToWKB(writer, iobuff, geo, &wkb_size);
        //printf("Wkb Size %d\n", wkb_size);

        if (l->gid != -1)
            GEOSGeom_destroy(geo);

        // send l.wkb to all histogram cells that intersects
		bool cloned = false;
		GList *cells = ds->metadata.hist.intersects(ds, l->mbr);
		g_list_foreach(cells, c) {
			histogram_cell *cell = (histogram_cell*)c->data;
			if (cell->place == 0) {
				printf("Cell: %d.%d cardin: %f objcount: %f points: %f\n", 
					cell->x, cell->y, cell->cardin, cell->objcount, cell->points);
				assert(cell->place != 0 && "Cell place needs to be defined\n");
			}
			SendWkb(cell->place, cell->x, cell->y, l->gid, cloned, wkb, wkb_size);
			cloned = true;
			//printf("cell %d\n", cell->place);
		}
		g_list_free(cells);

        read++;
        print_progress_gauge(read, ds->metadata.count);
    }

    GEOSDestroyWKBWritter(writer, iobuff);
    finishGEOS_r(geoscontext);
}


