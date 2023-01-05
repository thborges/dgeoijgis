
#include <string.h>
#include "system.h"
#include "ogrext.h"
#include <utils.h>
#include <dataset.h>
#include <multiway-join.h>
#include <histcommon.h>
#include "goheader.h"

int jcondcount;
char *jconditions[100];
GList *joinplan;
void *lastjoinpredicate;
GList *datasets = NULL;
enum HistogramType htype = HIST_GRID;
OGRwkbGeometryType ds_type = 0;


void run_system_cmd(char *cmd) {
        cmd[0] = ' ';
        cmd[strlen(cmd)-1] = ' ';
        fprintf(stderr, "Running system command: %s\n", cmd);
        system(cmd);
}

dataset *dataset_exists(const char *dataset_name) {
	//TODO: Slow for many datasets
	for(GList *i = datasets; i != NULL; i = g_list_next(i)) {
		dataset *d = i->data;
		if (strcmp(d->metadata.name, dataset_name) == 0)
			return d;
	}

	return NULL;
}

void clean_intermediates() {
	for(GList *i = datasets; i != NULL; i = g_list_next(i)) {
		dataset *d = i->data;
		// reset the intermediate copy of cells
		d->metadata.hist.distribute(d);
	}
	CleanIntermediate();	
}

void print_header_dataset() {
	printf("%10s|%10s|%10s|%10s|%10s|%10s|%10s|%10s|%11s|%10s| %s\n", "Items", "Pages", "X_Average", "Y_Average", "X Range", "Y Range", "X stddev", "Y stddev", "Extent Area", "Time (ms)", "File");
}

void print_header_index() {
	printf("%4s|%4s|%10s|%10s|%10s|%10s|%10s|%10s|%10s|%10s| %s\n", "M", "Clus", "Items", "Dirs", "Leaves", "Indexed", "Fill", "OverlapT", "OverlapM", "Time (ms)", "Name");
}

void distribute_data_through_servers(dataset *ds);

void add_new_dataset(dataset *ds) {
	datasets = g_list_prepend(datasets, ds);
}

void clean_local_datasets() {
	for(GList *i = datasets; i != NULL; i = g_list_next(i)) {
		dataset *d = i->data;
		dataset_destroy_full(d, TRUE);
	}
	g_list_free(datasets);
	datasets = NULL;
}

bool run_load_action(const char *shpfile, const char *dataset_name, bool prepare, int servers, bool simulate_load) {

	if (dataset_exists(dataset_name)) {
		fprintf(stderr, "error: Dataset %s alread exists.", dataset_name);
		return false;
	}

	OGRDataSourceH ds;
	ds = OGROpen(shpfile, FALSE, NULL);
	if (ds == NULL) {
		fprintf(stderr, "error: Invalid geo file.\n");
		return false;
	}

	OGRLayerH layer = OGR_DS_GetLayer(ds, 0);
	OGR_L_ResetReading(layer);
	int layer_count = OGR_L_GetFeatureCount(layer, FALSE);

	struct timespec cs;
	clock_gettime(CLOCK_REALTIME, &cs);
	
	OGRFeatureH feature;
	OGRGeometryH geometry;
	
	dataset *new_dataset = dataset_create(dataset_name, 1);
	new_dataset->temp_ogr_layer = layer;
	new_dataset->metadata.servers = servers > 0 ? servers : ActivePeersCount();
	printf("Active servers: %d\n", new_dataset->metadata.servers);
	if (servers > 0)
		printf("Simulated/used servers: %d\n", servers);

	if (ds_type != 0)
		new_dataset->metadata.hist.geom_type = ds_type;
	else
		new_dataset->metadata.hist.geom_type = OGR_GT_Flatten(OGR_L_GetGeomType(layer));
	printf("Geom type: %d\n", new_dataset->metadata.hist.geom_type);

	unsigned read = 0;
	while((feature = OGR_L_GetNextFeature(layer)) != NULL) {
		geometry = OGR_F_GetGeometryRef(feature);
		if (geometry != NULL) {
			dataset_leaf *leaf = dataset_add(new_dataset);
			long long gid = OGR_F_GetFID(feature);
			Envelope mbr = OGRGetEnvelope(geometry);
			dataset_fill_leaf_id(leaf, 0, gid, &mbr);
			leaf[0].points = OGRGetNumPoints(geometry);
		}
		OGR_F_Destroy(feature);

		read++;
		print_progress_gauge(read, layer_count);
	}

	enum HistogramHashMethod hm;
	char *estimate_method = getenv("MW_ESTIMATE_METHOD");
	if (estimate_method && strcmp(estimate_method, "MP") == 0)
		hm = HHASH_MBRCENTER;
	else
		hm = HHASH_AREAFRAC;

	histogram_generate_grid(new_dataset, hm, 0);	

	new_dataset->metadata.hist.distribute(new_dataset);
	
	if (!simulate_load) {
		int size = 0;
		dataset_histogram_persist *data = new_dataset->metadata.hist.get_data(new_dataset, &size);
		NewDataset(dataset_name, data, size, 0);
		distribute_data_through_servers(new_dataset);
		EndDataset();
		g_free(data);
	}

	//histogram_print(new_dataset, CARDIN);
	//histogram_print(new_dataset, POINTS);
	//histogram_print(new_dataset, PLACES);
	//new_dataset->metadata.hist.print_geojson(new_dataset);

	add_new_dataset(new_dataset);


	struct timespec cf;
	clock_gettime(CLOCK_REALTIME, &cf);
	double diff = runtime_diff_ms(&cs, &cf);

	new_dataset->temp_ogr_layer = NULL;
	OGR_DS_Destroy(ds);

	print_header_dataset();

	printf("%'10d|%10d|%10.5f|%10.5f|%10.1f|%10.1f|%10.5f|%10.5f|%11.1f|%10.0f| %s\n", 
		new_dataset->metadata.count,
		new_dataset->metadata.pagecount,
		new_dataset->metadata.x_average,
		new_dataset->metadata.y_average,
		new_dataset->metadata.mbr.MaxX - new_dataset->metadata.mbr.MinX,
		new_dataset->metadata.mbr.MaxY - new_dataset->metadata.mbr.MinY,
		dataset_meta_stddev(new_dataset->metadata, x),
		dataset_meta_stddev(new_dataset->metadata, y),
		ENVELOPE_AREA(new_dataset->metadata.mbr),
		diff, shpfile);

	return true;
}

bool run_distr_join_action(GList *joinplan, bool only_plan, void (*plan_action)(GList *, bool)) {
	// validate parameters
	multiway_input_chain* chain = (multiway_input_chain*)((GList*)g_list_next(joinplan))->data;
	enum JoinAlgorithm nextalg = g_list_length(joinplan) >= 2 ? chain->algorithm : join_none;
	dataset *ds;

	for(GList *l = joinplan; l != NULL; l = g_list_next(l)) {
		multiway_input_chain *i = l->data;
		switch (i->algorithm) {
			case join_hj:
			case join_p_hj:
				nextalg = i->algorithm;
				ds = dataset_exists(i->name);
				if (ds)
					i->dataset = ds;
				break;

			case join_inl:
			case join_p_inl:
			case join_rj:
			case join_p_rj:
				nextalg = i->algorithm;
				printf("error: Distributed select with rtree not implemented\n");
				return false;
				break;

			case join_none:
				switch (nextalg) {
					case join_inl:
					case join_p_inl:
					case join_hj:
					case join_p_hj:
						ds = dataset_exists(i->name);
						if (ds)
							i->dataset = ds;
						break;

					case join_rj:
					case join_p_rj:
						printf("error: Distributed select with rtree not implemented\n");
						return false;
						break;
					case join_none:
						printf("error: Malformed select\n");
						return false;
						break;
				}
				break;
		}

		if (!i->rtree) { // || i->dataset
			fprintf(stderr, "error: Dataset/Index %s not found.\n", i->name);
			return false;
		}
	}

	// do the work!
	plan_action(joinplan, only_plan);

	fflush(stdout);
	fflush(stderr);

	return true;
}

