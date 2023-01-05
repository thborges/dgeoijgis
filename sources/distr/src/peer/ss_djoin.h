
#ifndef DJOIN_H
#define DJOIN_H

#include <glibwrap.h>

typedef struct {
	dataset *ds;
	dataset_segment *sseg;
} go_dataset_seg;

unsigned add_glist_to_seg(dataset *result, dataset_segment *result_seg, GList *leaves);

GList *djoin_compare_segs(dataset *dr, dataset_segment *rseg, unsigned start, unsigned end,
	go_dataset_seg *dsc, int s_count, bool lastlevel, GEOSContextHandle_t geoscontext);

#endif
