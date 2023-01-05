
#ifndef SS_DSCOPY
#define SS_DSCOPY

typedef struct {
	GEOSContextHandle_t geoscontext;
	GEOSWKBWriter *writer;
	void *iobuff;
} copy_context;

typedef struct {
	GEOSWKTWriter *writer;
	void *iobuff;
} copy_context_wkt;

copy_context *start_copy_context();
copy_context_wkt *start_copy_context_wkt();
void end_copy_context(copy_context *cc);
void end_copy_context_wkt(copy_context_wkt *cc);
const char *get_wkb(copy_context *cc, dataset_leaf *l, int *wkb_size, long long *gid, bool *is_copy);
void add_geoitem(dataset *ds, dataset_segment *seg, long long gid, bool IsCopy, unsigned char *wkb, unsigned wkb_len, Envelope *mbr);
const char *get_wkt(copy_context_wkt *cc, dataset_leaf *l, int index, long long *gid, bool *is_copy);
const char *get_histo_cell_wkt(dataset *ds, unsigned short x, unsigned short y);
bool filtered(Envelope fenv, int count, dataset_leaf *l);

#endif
