
#include <geosext.h>
#include <dataset.h>
#include <ss_dscopy.h>

char wkt_cell_histogram[1024];

copy_context *start_copy_context() {
	copy_context *cc = g_new(copy_context, 1);
	cc->geoscontext = initGEOS_r(geos_messages, geos_messages);
	cc->writer = GEOSCreateWKBWritter(cc->geoscontext);
	cc->iobuff = GEOSCreateWKBBuffer();
	return cc;
}

void end_copy_context(copy_context *cc) {
    GEOSDestroyWKBWritter(cc->writer, cc->iobuff);
    finishGEOS_r(cc->geoscontext);
    g_free(cc);
}

copy_context_wkt *start_copy_context_wkt() {
	copy_context_wkt *cc = g_new(copy_context_wkt, 1);
	cc->writer = GEOSCreateWKTWritter();
	cc->iobuff = GEOSCreateWKTBuffer();
	return cc;
}

void end_copy_context_wkt(copy_context_wkt *cc) {
	GEOSDestroyWKTWritter(cc->writer, cc->iobuff);
    g_free(cc);
}

const char *get_wkb(copy_context *cc, dataset_leaf *l, int *wkb_size, long long *gid, bool *is_copy) {
	//printf("Copying wkb\n");
	*is_copy = l[0].cloned;
	*gid = l[0].gid;
	return GEOSGeomToWKB(cc->writer, cc->iobuff, l[0].geo, wkb_size);

}

void add_geoitem(dataset *ds, dataset_segment *seg, long long gid, bool IsCopy, unsigned char *wkb, unsigned wkb_len, Envelope *mbr) {
	//pwkb := (*C.uchar)(unsafe.Pointer(&wkb[0]))
	GEOSGeometryH ggeo = GEOSGeomFromWKB_buf(wkb, wkb_len);
	if (ggeo == NULL) {
		printf("Error converting WKB to GEOS: %s", ds->metadata.name);
	}

	dataset_leaf *leaves = dataset_add_seg(ds, seg);
	dataset_fill_leaf_ext(leaves, 0, gid, IsCopy, ggeo, NULL, NULL);
}

const char *get_wkt(copy_context_wkt *cc, dataset_leaf *l, int index, long long *gid, bool *is_copy) {
	dataset_leaf *leaf = &l[index];
	*is_copy = leaf->cloned;
	*gid = leaf->gid;
	const char *wkt = DgeoGeomToWKT(cc->writer, cc->iobuff, leaf->geo);
	return wkt;
}

bool filtered(Envelope fenv, int count, dataset_leaf *l) {
	for(int i = 0; i < count; i++) {
		if (ENVELOPE_INTERSECTS(fenv, l[i].mbr))
			return 0;
	}
	return 1;
}

const char *get_histo_cell_wkt(dataset *ds, unsigned short x, unsigned short y) {
	dataset_histogram *h = &ds->metadata.hist;
	histogram_cell *cell = h->get_cell(h, x, y);
	Envelope e = h->get_cell_envelope(h, cell);
	sprintf(wkt_cell_histogram, "POLYGON((%f %f, %f %f, %f %f, %f %f, %f %f))",
		e.MinX, e.MinY, 
		e.MaxX, e.MinY,
		e.MaxX, e.MaxY,
		e.MinX, e.MaxY,
		e.MinX, e.MinY);
	return wkt_cell_histogram;
}

