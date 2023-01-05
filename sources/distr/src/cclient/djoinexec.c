#include <string.h>
#include <glibwrap.h>
#include <multiway-join.h>
#include <dataset.h>
#include <utils.h>
#include <sys/queue.h>
#include "goheader.h"
#include "system.h"

void execute_join_opt(const char *query_name, int levels, int servers, optimization_data_level *opt_data_query);

decl_qsort_p_cmp(sort_optdata_dependencies_decreasing, a, b, thunk) {
	optimization_data_s *ia = (optimization_data_s*)a;
	optimization_data_s *ib = (optimization_data_s*)b;
	dataset_histogram *hr = (dataset_histogram*)thunk;

	histogram_cell *ca = hr->get_cell(hr, ia->xl, ia->yl);
	histogram_cell *cb = hr->get_cell(hr, ib->xl, ib->yl);

	int dep_copy_a = __builtin_popcount(ca->copies);
	int dep_copy_b = __builtin_popcount(cb->copies);

	int dep_a = 0;
	if (ia->lcell->place != ca->place) dep_a++;
	for(int i=0; i < ia->rcells_size; i++)
		if (ia->rcells[i].cell->place != ca->place) dep_a++;

	int dep_b = 0;
	if (ib->lcell->place != cb->place) dep_b++;
	for(int i=0; i < ib->rcells_size; i++)
		if (ib->rcells[i].cell->place != cb->place) dep_b++;

	/* first ordering is by dependencies: how many partitions will be copied
		 second ordering is by the number of copies, i.e., other servers that will need it
		   third ordering is by xl, yl
	*/
	if (dep_a == dep_b) {
		if (dep_copy_a == dep_copy_b) {
			if (ia->xl == ib->xl) {
				if (ia->yl > ib->yl) return 1;
				if (ia->yl < ib->yl) return -1;
				return 0;
			} else {
				if (ia->xl > ib->xl) return 1;
				return -1;
			}
		}
		if (dep_copy_a > dep_copy_b) return 1;
		return -1;
	}
	if (dep_a > dep_b) return -1;
	return 1;
}

int sort_optdata_increasing(const void *x, const void *y) {
	optimization_data_s *iy = (optimization_data_s*)y;
	optimization_data_s *ix = (optimization_data_s*)x;
	if (ix->pnts == iy->pnts) return 0;
	if (ix->pnts > iy->pnts) return 1;
	return -1;
}

void get_query_name(char *query_name, int max) {
	char *query_fn = getenv("MW_FILENAME");
	if (query_fn == NULL)
		query_fn = "inputquery.txt";
	int len = 0;
	while (query_fn[len] != 0) {
		if (query_fn[len] == '.') break;
		len++;
	}
	if (len >= max) len = max-1;
	strncpy(query_name, query_fn, len);
	query_name[len] = '\0';
}

void djoin_execute(GList *joinplan, bool only_plan) {

	struct timespec cs;
	clock_gettime(CLOCK_REALTIME, &cs);

	char query_name[100];
	get_query_name(query_name, 100);
	char *qname = getenv("MW_QNAME");

	// optimization method results
	FILE *fresults = NULL;
	if (verbose) {
		fresults = fopen("opt_results.txt", "wb");
		fprintf(fresults, "[%s]\n", query_name);
		fprintf(fresults, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", "step", "method", "pairs", "servers", "mksp", "comm", "mksp_sdev", "time");
	}
	else if (!printed_nonverbose_header) {
		printf("Plan results\n");
		printf("%12s %12s %12s %12s %5s %12s\n", "Effort", "Comm", "Mkspan", "Stdev Mksp", "MGap%", "ms");
		printed_nonverbose_header = true;
	}

	// start with first dataset as left input
	multiway_input_chain li = *(multiway_input_chain*) joinplan->data;

	// get the total number of servers on datasets and levels
	unsigned char levels = 0;
	int servers_count = 0;
	for (GList *next_input = joinplan; next_input != NULL; next_input =
			g_list_next(next_input)) {
		multiway_input_chain *ri = next_input->data;
		servers_count = MAX(servers_count, ri->dataset->metadata.servers);
		levels++;
	}
	levels--; //don't count the from clause

	multiway_histogram_estimate estimate[servers_count+1];
	multiway_histogram_estimate estimate_per_server[servers_count+1];
	memset(estimate_per_server, 0, sizeof(multiway_histogram_estimate) * (servers_count+1));
	optimization_data_level opt_data_query[levels];

	dataset *jresults = NULL;
	dataset *interm_datasets[levels];
	unsigned char level = 0;

	for (GList *next_input = g_list_next(joinplan);
		 next_input != NULL;
		 next_input = g_list_next(next_input)) {

		multiway_input_chain ri = *(multiway_input_chain*)next_input->data;

		enum JoinPredicateCheck next_pcheck = CHECKR;
		GList *next_next = g_list_next(next_input);
		if (next_next != NULL) {
			multiway_input_chain *nextjoin = next_next->data;
			next_pcheck = nextjoin->pcheck;
		}

		dataset_histogram newhisto;

		switch (ri.algorithm) {
		case join_hj:
			// put the next check on the left
			if (next_pcheck != CHECKR) {
				multiway_input_chain aux = li;
				li = ri;
				ri = aux;
			}

			memset(estimate, 0, sizeof(multiway_histogram_estimate) * (servers_count+1));

			newhisto = do_join_over_hist(li.dataset, ri.dataset, level, estimate, 
				estimate_per_server, &opt_data_query[level], fresults);

			char aux_name[200];
			sprintf(aux_name, "%s_step%d", query_name, level); 
			
			jresults = dataset_create_mem(aux_name, 2);
			jresults->metadata.mbr = li.dataset->metadata.mbr;
			jresults->metadata.servers = MAX(ri.dataset->metadata.servers, li.dataset->metadata.servers);
			jresults->metadata.x_average = li.dataset->metadata.x_average;
			jresults->metadata.y_average = li.dataset->metadata.y_average;
			jresults->metadata.hist = newhisto;
			interm_datasets[level] = jresults;

			// tell servers the existence of the intermediate dataset,
			// and send the histogram data
			if (!only_plan) {
				int size = 0;
				dataset_histogram_persist *data = jresults->metadata.hist.get_data(jresults, &size);
				NewDataset(jresults->metadata.name, data, size, 1);
				EndDataset();
				g_free(data);
			}

			break;

		default:
			fprintf(stderr, "Join type not implemented.\n");
			break;
		}

		li.name = jresults->metadata.name;
		li.dataset = jresults;

		//histogram_print(jresults, CARDIN);
		//histogram_print(jresults, POINTS);
		//histogram_print(jresults, PLACES);
		//jresults->metadata.hist.print_geojson(jresults);

		level++;

		// agregate estimates
		for(int i = 1; i <= servers_count; i++) {
			estimate_per_server[i].to_pnts += estimate[i].to_pnts;
			estimate_per_server[i].io_objs += estimate[i].io_objs;
			estimate_per_server[i].io_pnts += estimate[i].io_pnts;
			estimate_per_server[i].io_save += estimate[i].io_save;
			estimate_per_server[i].inters += estimate[i].inters;
		}
	}

	struct timespec cf;
	clock_gettime(CLOCK_REALTIME, &cf);
	double runtime = runtime_diff_ms(&cs, &cf);

	double totalpnts, totalcomm, mksp, stdevmksp, mksp_gap;
	multiway_totalize_estimate(estimate_per_server, servers_count, &totalpnts, &totalcomm, 
		&mksp, NULL, &stdevmksp, NULL, &mksp_gap);

	if (verbose) {
		printf("\nESTIMATE PER SERVER, AGGREGATED\n");
		multiway_print_estimate("Server", estimate_per_server, servers_count);

		printf("\nPLAN RUNTIME: \033[91m%'10.0f ms \033[0m\n", runtime);

		fprintf(fresults, "total\t\t\t\t%12.0f\t%12.0f\t%12.0f\t%10.2f\n", mksp, totalcomm, stdevmksp, runtime);
		fclose(fresults);
	}
	else {
		printf("%12.0f %12.0f %12.0f %12.0f %5.2f %12.0f\tTT\n", totalpnts, totalcomm, mksp, 
			stdevmksp, mksp_gap, runtime);
	}

	if (!only_plan) {
		execute_join_opt(query_name, levels, servers_count, opt_data_query);
	}

	for(int l = 0; l < levels; l++) {
		int opt_atu = opt_data_query[l].opt_data_size;
		optimization_data_s *opt_data = opt_data_query[l].opt_data; 
		free_opt_data(opt_data, opt_atu);
		dataset_destroy_full(interm_datasets[l], TRUE);
	}
}


STAILQ_HEAD(server_job_queue, server_job_entry);
struct server_job_entry {
    STAILQ_ENTRY(server_job_entry) entries;
    int id;
};

double send_job_to_server(optimization_data_s *opt_data, dataset *dl, dataset *dr,
	dataset_histogram *hl, dataset_histogram *resulth, int level) {

	int xl = opt_data->xl;
	int yl = opt_data->yl;
	histogram_cell *result_cell = resulth->get_cell(resulth, xl, yl);

	histogram_cell *lcell = hl->get_cell(hl, xl, yl);
	StartJoinMsg(dl->metadata.name, xl, yl, lcell->place, level);
	if (lcell->place == 0) {
		printf("Cell %d.%d from %s: card %f, points %f\n",
			xl, yl, dl->metadata.name, lcell->cardin, lcell->points);
		assert(lcell->place != 0 && "Place needs to be defined by optimizer.");
	}

	// Check the predicate
	for(int r = 0; r < opt_data->rcells_size; r++) {
		right_opt_data rcell = opt_data->rcells[r];
		if (rcell.cell->place == 0) {
			printf("Cell %d.%d from %s: card %f, points %f\n",
				rcell.xr, rcell.yr, dr->metadata.name, 
				rcell.cell->cardin, rcell.cell->points);
			assert(rcell.cell->place != 0 && "Place needs to be defined by optimizer.");
		}
		AddRToJoinMsg(dr->metadata.name, rcell.xr, rcell.yr, rcell.cell->place);
	}
	
	EndJoinMsg(result_cell->place);
	return result_cell->cardin;
}

void execute_join_opt(const char *query_name, int levels, int servers, optimization_data_level *opt_data_query) {
	// start a new distributed join
	struct timespec cs;
	clock_gettime(CLOCK_REALTIME, &cs);

	NewJoin(query_name, levels);

	for(int l = 0; l < levels; l++) {
		int opt_atu = opt_data_query[l].opt_data_size;
		optimization_data_s *opt_data = opt_data_query[l].opt_data; 
		dataset *dl = opt_data_query[l].dl;
		dataset *dr = opt_data_query[l].dr;
		dataset_histogram *hl = &dl->metadata.hist;
		dataset_histogram *resulth = &opt_data_query[l].result;

		// sort in decreasing order of dependencies 
		qsort_p(opt_data, opt_atu, sizeof(optimization_data_s), sort_optdata_dependencies_decreasing, resulth);

		int turn[servers+1];
		struct server_job_queue server_jobs[servers+1];
		for(int s = 1; s <= servers; s++) {
			STAILQ_INIT(&server_jobs[s]);
			turn[s] = 100; // 100 fist jobs without dependencies to fill the predicate queue
		}

		// separate jobs per server
		for(int j=0; j < opt_atu; j++) {
			histogram_cell *result_cell = resulth->get_cell(resulth, opt_data[j].xl, opt_data[j].yl);

	        struct server_job_entry *job = g_new(struct server_job_entry, 1);
			job->id = j;
			assert(result_cell->place > 0);
			STAILQ_INSERT_TAIL(&server_jobs[result_cell->place], job, entries);
		}

		double estimated_cardin = 0;
		int sent = 1;
		while (sent > 0) {
			sent = 0;
			for(int s=1; s <= servers; s++) {
				if (STAILQ_EMPTY(&server_jobs[s]))
					continue;

				int c;
				struct server_job_entry *job;
				if (turn[s] > 0) {
					job = STAILQ_LAST(&server_jobs[s], server_job_entry, entries);
					c = job->id;
					turn[s]--;
				} else {
					job = STAILQ_FIRST(&server_jobs[s]);
					c = job->id;
					turn[s]++;
				}
				STAILQ_REMOVE(&server_jobs[s], job, server_job_entry, entries);
				g_free(job);

				estimated_cardin += send_job_to_server(&opt_data[c], dl, dr, hl, resulth, l);
				sent++;
			}
		}
		fprintf(stderr, "Estimated cardinality level %d: %f\n", l, estimated_cardin);
	}

	// Signal join end and wait for finish
	printf("Waiting join end.\n");
	EndJoin(levels);

	struct timespec cf;
	clock_gettime(CLOCK_REALTIME, &cf);
	double diff = runtime_diff_ms(&cs, &cf);
	printf("Execution\033[91m: %'10.0f ms \033[0m\n", diff);
}

