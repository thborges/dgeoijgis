#include <utils.h>
#include <dataset.h>
#include <ogrext.h>
#include "ss_djoin.h"

unsigned add_glist_to_seg(dataset *result, dataset_segment *result_seg, GList *leaves) {
	unsigned r = 0;
	for(GList *cur = g_list_first(leaves); cur != NULL; cur = g_list_next(cur)) {
		dataset_leaf* leaf = dataset_add_seg(result, result_seg);
		leaf[0] = *(dataset_leaf*)cur->data;
		r++;
	}
	g_list_free_full(leaves, g_free);
	return r;
}

GList *djoin_compare_segs(dataset *dr, dataset_segment *rseg, unsigned start, unsigned end,
	go_dataset_seg *dsc, int s_count, bool lastlevel, GEOSContextHandle_t geoscontext) {

	int r = -1;
	GList *result = NULL;
	
	dataset_iter_seg ir;
	dataset_foreach_seg(ir, dr, rseg)
	{
		r++;
		if (r < start) continue;
		if (r >= end) {
            dataset_release_iter(&ir);
            break;
        }

		//printf("Dataset r: %s, %d, %d - s_count %d\n", dr->metadata.name, xr, yr, s_count);
		dataset_leaf *rgeo = get_join_pair_leaf(ir.item, CHECKR);
		const GEOSPreparedGeometry *pgeo = GEOSPrepare_r(geoscontext,
				rgeo->geo);

		for (int satu = 0; satu < s_count; satu++) {
			//printf("Dataset s: %s, %d, %d\n", ds->metadata.name, dsc[satu].xs, dsc[satu].ys);

			dataset *ds = dsc[satu].ds;
			dataset_segment *sseg = dsc[satu].sseg;

			dataset_iter_seg is;
			dataset_foreach_seg(is, ds, sseg)
			{
				dataset_leaf *sgeo = get_join_pair_leaf(is.item, CHECKR);

				if (!ENVELOPE_INTERSECTS(rgeo->mbr, sgeo->mbr)) {
					continue;
				}

				double MinX = MAX(rgeo->mbr.MinX, sgeo->mbr.MinX);
				double MinY = MAX(rgeo->mbr.MinY, sgeo->mbr.MinY);

				unsigned short xrtest, yrtest;
				unsigned sid;

				histogram_cell *refcell = dr->metadata.hist.get_reference_cell(dr, MinX, MinY);
				xrtest = refcell->x;
				yrtest = refcell->y;
				//if (lastlevel) {
					sid = xrtest + (yrtest << 16);
					if (rseg->sid != sid) {
						continue;
					}
				//}

				unsigned short xtest, ytest;
				refcell = ds->metadata.hist.get_reference_cell(ds, MinX, MinY);
				xtest = refcell->x;
				ytest = refcell->y;
				sid = xtest + (ytest << 16);
				if (sseg->sid != sid) {
					continue;
				}

				if (GEOSPreparedIntersects_r(geoscontext, pgeo, sgeo->geo)) {
					//GEOSIntersects_r(geoscontext, rgeo->geo, sgeo->geo)) {

                    // the dataset page could be out of memory when adding items
                    // to the final result dataset, in add_glist_to_seg
                    dataset_leaf *leaf = g_new(dataset_leaf, 1);
                    *leaf = *rgeo;

					// trim envelope to make Interior Point method report 
					// this result in posterior steps of multiway
					//leaf->mbr.MinX = MAX(leaf->mbr.MinX, dr->metadata.hist.mbr.MinX + dr->metadata.hist.xsize * xrtest);
					//leaf->mbr.MinY = MAX(leaf->mbr.MinY, dr->metadata.hist.mbr.MinY + dr->metadata.hist.ysize * yrtest);
					result = g_list_prepend(result, leaf);
				}
			}
		}

		GEOSPreparedGeom_destroy_r(geoscontext, pgeo);
	}

	return result;
}
