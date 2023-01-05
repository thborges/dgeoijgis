
#include <glibwrap.h>
#include <multiway-join.h>
#include <dataset.h>
#include <utils.h>
#include <limits.h>
#include <system.h>
#include <math.h>

void lp_optimize_hr(dataset_histogram *hr, int servers,
	optimization_data_s *opt_data, int opt_atu, multiway_histogram_estimate *agg_sever,
	double f, double dualvalues[opt_atu]);

void bs_optimize_hr(dataset_histogram *hr, int servers,
	optimization_data_s *opt_data, int opt_atu, multiway_histogram_estimate *agg_sever,
	double f);

void lagrange_sm_optimize_hr(dataset_histogram *hr, int servers,
	optimization_data_s *opt_data, int opt_atu, multiway_histogram_estimate *agg_sever,
	double f, double dualvalues[opt_atu]);

void lpi_sm_optimize_hr(dataset_histogram *hr, int servers,
	optimization_data_s *opt_data, int opt_atu, multiway_histogram_estimate *agg_server,
	double f, bool only_root_node);

void totalize_estimate(dataset_histogram *hr, multiway_histogram_estimate *estimate, 
	int servers, optimization_data_s *opt_data, int pairs);

void reset_opt_data_copies(dataset_histogram *hr, optimization_data_s *opt_data, int opt_atu);
 
void persist_opt_data(dataset_histogram *hr, Envelope lmbr, optimization_data_s *opt_data, int opt_atu, int servers, int step, multiway_histogram_estimate *agg_server, double f);

double estimate_intersections_mamoulis_papadias(Envelope el, Envelope er, Envelope inters, 
	histogram_cell *lcell, histogram_cell *rcell, double *qtdobjl_out) {
	// the code below follows equations (1) and (2) in Mamoulis, Papadias 2001

	// estimate the quantity of objects in LeftDs in the inters window: eqn (1)
	double ux = el.MaxX - el.MinX;
	double uy = el.MaxY - el.MinY;
	double avgl_x = OLD_AVG_LENGTH_X(lcell);
	double avgl_y = OLD_AVG_LENGTH_Y(lcell);
	double wx = inters.MaxX - inters.MinX;
	double wy = inters.MaxY - inters.MinY;
	double qtdobjl = lcell->cardin * 
		MIN(1,(avgl_x+wx)/ux) * 
		MIN(1,(avgl_y+wy)/uy);
	(*qtdobjl_out) = qtdobjl;

	// estimate the quantity of objects in RightDs in the inters window eqn (1)
	ux = er.MaxX - er.MinX;
	uy = er.MaxY - er.MinY;
	double avgr_x = OLD_AVG_LENGTH_X(rcell);
	double avgr_y = OLD_AVG_LENGTH_Y(rcell);
	double qtdobjr = rcell->cardin * 
		MIN(1,(avgr_x+wx)/ux) * 
		MIN(1,(avgr_y+wy)/uy);

	// estimate join result cardinality, eqn (2)
	return qtdobjl * qtdobjr * MIN(1, (avgl_x + avgr_x)/wx) * MIN(1, (avgl_y + avgr_y)/wy);
}

double estimate_intersections_zu_without_axis_overlap(Envelope el, Envelope er, 
	Envelope inters, histogram_cell *lcell, histogram_cell *rcell, 
	dataset_histogram *dh_l, dataset_histogram *dh_r) {

	if (!ENVELOPE_INTERSECTS(lcell->usedarea, rcell->usedarea))
		return 0;

	Envelope w = EnvelopeIntersection2(lcell->usedarea, rcell->usedarea);
	double wx = w.MaxX - w.MinX;
	double wy = w.MaxY - w.MinY;

	bool line_to_line = false;
	bool line_to_polygon = false;
	bool left_is_line = false;
	if ((dh_l->geom_type == wkbLineString || dh_l->geom_type == wkbMultiLineString) &&
        (dh_r->geom_type == wkbPolygon    || dh_r->geom_type == wkbMultiPolygon)) {
		line_to_polygon = true;
		left_is_line = true;
	}
	else if ((dh_r->geom_type == wkbLineString || dh_r->geom_type == wkbMultiLineString) &&
        	 (dh_l->geom_type == wkbPolygon    || dh_l->geom_type == wkbMultiPolygon)) {
		line_to_polygon = true;
		left_is_line = false;
	}
	else if ((dh_l->geom_type == wkbLineString || dh_l->geom_type == wkbMultiLineString) && 
			 (dh_r->geom_type == wkbLineString || dh_r->geom_type == wkbMultiLineString)) {
		line_to_line = true;
		left_is_line = true;
	}

	// estimate the quantity of objects in LeftDs inside inters window
	double ux_l = el.MaxX - el.MinX;
	double uy_l = el.MaxY - el.MinY;
	double avgl_x = OLD_AVG_LENGTH_X(lcell);
	double avgl_y = OLD_AVG_LENGTH_Y(lcell);

	// observing that objects generally doesn't overlap in both axis,
	// 	fix the probability of intersection in one of them
	double usx_l = lcell->usedarea.MaxX - lcell->usedarea.MinX;
	double usy_l = lcell->usedarea.MaxY - lcell->usedarea.MinY;
	if (avgl_x > avgl_y)
		avgl_x = MIN(usx_l/(avgl_y*lcell->objcount/usy_l), avgl_x);
	else
		avgl_y = MIN(usy_l/(avgl_x*lcell->objcount/usx_l), avgl_y);

	double qtdobjl = MAX(0.0, lcell->cardin) * 
		MIN(1.0,(avgl_x + wx)/usx_l) * 
		MIN(1.0,(avgl_y + wy)/usy_l);

	// estimate the quantity of objects in RightDs inside inters window
	double ux_r = er.MaxX - er.MinX;
	double uy_r = er.MaxY - er.MinY;
	double avgr_x = OLD_AVG_LENGTH_X(rcell);
	double avgr_y = OLD_AVG_LENGTH_Y(rcell);

	// observing that objects generally doesn't overlap in both axis,
	// 	fix the probability of intersection in one of them
	double usx_r = rcell->usedarea.MaxX - rcell->usedarea.MinX;
	double usy_r = rcell->usedarea.MaxY - rcell->usedarea.MinY;
	if (avgr_x > avgr_y)
		avgr_x = MIN(usx_r/(avgr_y*rcell->objcount/usy_r), avgr_x);
	else
		avgr_y = MIN(usy_r/(avgr_x*rcell->objcount/usx_r), avgr_y);

	double qtdobjr = MAX(0.0, rcell->cardin) * 
		MIN(1.0,(avgr_x + wx)/usx_r) * 
		MIN(1.0,(avgr_y + wy)/usy_r);

	// estimate join result cardinality
	double result = qtdobjl * qtdobjr * 
		MIN(1.0, (avgl_x + avgr_x)/wx) * 
		MIN(1.0, (avgl_y + avgr_y)/wy);

	double coef_area = 0;
	if (line_to_polygon) {
		/* disabling this increases the estimation a litle bit for J1,J2
		avgr_x = MIN(wx,avgr_x);
		avgr_y = MIN(wy,avgr_y);
		avgl_x = MIN(wx,avgl_x);
		avgl_y = MIN(wy,avgl_y);*/

		if (left_is_line) {
			double d = rcell->areasum/(ux_r*uy_r);
			double f = sqrt(((ux_r*uy_r)/rcell->objcount)/(avgr_x*avgr_y));
			double navgx = avgr_x * f;
			double navgy = avgr_y * f;
			result = qtdobjl * d * MAX(1.2,avgl_x / navgx) * MAX(1.2,avgl_y / navgy);
		} else {
			double d = lcell->areasum/(ux_l*uy_l);
			double f = sqrt(((ux_l*uy_l)/lcell->objcount)/(avgl_x*avgl_y));
			double navgx = avgl_x * f;
			double navgy = avgl_y * f;
			result = qtdobjr * d * MAX(1.2,avgr_x / navgx) * MAX(1.2,avgr_y / navgy);
		}
	}
	else if (line_to_line) {
		/*
			The probability of two random line segments intersect in unit square = 1/3
			The probability of four random points forms a convex quadrilateral 133/144
			The overall probability then, 1/3 * 133/144 = 133/432 ~= 0.3078
			Source: math.stackexchange.com/questions/134525
		*/
		double line_coef;
		double margina = sqrt(pow(avgl_x,2) + pow(avgl_y,2));
		double marginb = sqrt(pow(avgr_x,2) + pow(avgr_y,2));
		if (margina < marginb)
			line_coef = MIN(133.0/432.0, margina/marginb);
		else
			line_coef = MIN(133.0/432.0, marginb/margina);
		result = qtdobjl * qtdobjr * line_coef *
			MIN(1.0, (avgl_x + avgr_x)/wx) *
			MIN(1.0, (avgl_y + avgr_y)/wy);
	}

	/*if (lcell->x == 7 && lcell->y == 1) {
		fprintf(stderr, "-----\n%d.%d x %d.%d = %f ", lcell->x, lcell->y, rcell->x, rcell->y, result);
		fprintf(stderr, "wx %f wy %f coef_area %f\n", wx, wy, coef_area);
		fprintf(stderr, "lcard %f rcard %f\n", lcell->cardin, rcell->cardin);
		fprintf(stderr, "avgl_x %f avgl_y %f, avgr_x %f avgr_y %f\n", avgl_x, avgl_y, avgr_x, avgr_y); 
		fprintf(stderr, "ul_x %f ul_y %f objcnt_l %f\n", ux_l, uy_l, lcell->objcount);
	*/
		/*fprintf(stderr, "L | x | y | xqtd | xavg     | yqtd | yavg     | qtdobj_l \n");
 		for(int bx=0; bx < AVGL_HISTO_SIZE; bx++) {
	 		for(int by=0; by < AVGL_HISTO_SIZE; by++) {
				fprintf(stderr, "  | %1d | %1d | %4d | %f | %4d | %f | %f\n", 
					bx, by, lcell->len[bx].qtd_x, lcell->len[bx].avg_x, 
					lcell->len[bx].qtd_y[by], lcell->len[bx].avg_y[by], qtdobjl[bx][by]);
			}
		}*/
 	//	fprintf(stderr, "ur_x %f ur_y %f objcnt_r %f\n", ux_r, uy_r, rcell->objcount);
		/*fprintf(stderr, "R | x | y | xqtd | xavg     | yqtd | yavg     | qtdobj_l \n");
		for(int bx=0; bx < AVGL_HISTO_SIZE; bx++) {
	 		for(int by=0; by < AVGL_HISTO_SIZE; by++) {
				fprintf(stderr, "  | %1d | %1d | %4d | %f | %4d | %f | %f\n", 
					bx, by, rcell->len[bx].qtd_x, rcell->len[bx].avg_x, 
					rcell->len[bx].qtd_y[by], rcell->len[bx].avg_y[by], qtdobjr[bx][by]);
			}
		}*/
	//}

	return result;
}

dataset_histogram do_join_over_hist(dataset *dl, dataset *dr, unsigned char level,
	multiway_histogram_estimate *estimate, multiway_histogram_estimate *agg_server,
	optimization_data_level *opt_data_level, FILE *fresults) {

	int servers = MAX(dl->metadata.servers, dr->metadata.servers)+1;
	printf("\n\n------- Starting optimization for level %d, servers %d -------\n\n", level, servers-1);

	// check if MW_F or MW_F0... is set
	char *auxf = getenv("MW_F");
	double f = auxf ? atof(auxf) : 1;
	char fstep[20];
	sprintf(fstep, "MW_F%d", level+1);
	auxf = getenv(fstep);
	if (auxf)
		f = atof(auxf);
	printf("Tradeoff f: %f\n",  f);

	char *estimate_method = getenv("MW_ESTIMATE_METHOD");
	if (!estimate_method)
		estimate_method = "ZU";

	if (verbose) {
		printf("L dataset: %s\n", dl->metadata.name);
		printf("R dataset: %s\n", dr->metadata.name);
	}

	dataset_histogram *hl = &dl->metadata.hist;
	dataset_histogram *hr = &dr->metadata.hist;

	dataset_histogram result;
	hl->clone_histogram(dl, &result);

	// temporary store the visited right cells to set PLACE
	int max_right_cells_int = 10;
	int cur_right_cells_int = 0;
	right_opt_data *right_cells_int;
	right_cells_int = g_new(right_opt_data, max_right_cells_int);

	int opt_size = 10;
	int opt_atu = 0;
	optimization_data_s *opt_data = g_new(optimization_data_s, opt_size);

	Envelope dlev;
	dlev.MinX = MAX(dl->metadata.mbr.MinX, dr->metadata.mbr.MinX);
	dlev.MinY = MAX(dl->metadata.mbr.MinY, dr->metadata.mbr.MinY);
	dlev.MaxX = MIN(dl->metadata.mbr.MaxX, dr->metadata.mbr.MaxX);
	dlev.MaxY = MIN(dl->metadata.mbr.MaxY, dr->metadata.mbr.MaxY);

	GList *dl_cells = hl->intersects(dl, dlev);
	g_list_foreach(dl_cells, dlcell) {

		histogram_cell *lcell = (histogram_cell*)dlcell->data;
		Envelope el = hl->get_cell_envelope(hl, lcell);

		double total_io[servers], total_objs[servers];
		memset(total_io, 0, sizeof total_io);
		memset(total_objs, 0, sizeof total_objs);
		cur_right_cells_int = 0;

		histogram_cell *resultcell = result.get_cell(&result, lcell->x, lcell->y);
		memcpy(resultcell->len, lcell->len, AVGL_HISTO_SIZE * sizeof(avglength2d));
		resultcell->usedarea = lcell->usedarea;
			
		// the lcell could be copied to any server, except its current place
		// double total_effort = lcell->points;
		double total_effort = 0;
		for(int s = 1; s < servers; s++) {
			if (lcell->place != s) {
				total_io[s] += lcell->points;
				total_objs[s] += lcell->cardin;
			}
		}

		GList *dr_cells = hr->intersects(dr, el);
		g_list_foreach(dr_cells, drcell) {

			histogram_cell *rcell = (histogram_cell*)drcell->data;

			if (lcell->objcount <= 0.0 || rcell->objcount <= 0.0)
				continue;

			Envelope er = hr->get_cell_envelope(hr, rcell);
			char i = ENVELOPE_INTERSECTS(el, er);
			if (i == 0) continue;

			// store the cell pointer to set place later
			if (cur_right_cells_int >= max_right_cells_int) {
				max_right_cells_int *= 2;
				right_cells_int = g_renew(right_opt_data, right_cells_int, max_right_cells_int);
			}
			right_cells_int[cur_right_cells_int].xr = rcell->x;
			right_cells_int[cur_right_cells_int].yr = rcell->y;
			right_cells_int[cur_right_cells_int].cell = rcell;
			cur_right_cells_int++;

			//total_points += lcell->objcount * (rcell->objcount * rcell->points);
			//total_points += rcell->points;

			// the right cell copy to any server different of its place
			for(int s = 1; s < servers; s++) {
				if (rcell->place != s) {
					total_io[s] += rcell->points;
					total_objs[s] += rcell->objcount;
				}
			}

			// Estimate the new histogram
			Envelope inters = EnvelopeIntersection2(el, er);
			double intarea = ENVELOPE_AREA(inters);
			if (intarea == 0)
				continue;

			double qtdobjl = 0;
			double intersections;
			if (strcmp(estimate_method, "MP") == 0)
				intersections = estimate_intersections_mamoulis_papadias(el, er, 
					inters, lcell, rcell, &qtdobjl);
			else {// ZU
				intersections = estimate_intersections_zu_without_axis_overlap(el, er,
					inters, lcell, rcell, hl, hr);
			}

			// sometimes the result of methods above returns a nan due a division by zero
			if (isnan(intersections))
				intersections = 0;

			total_effort += lcell->objcount * rcell->objcount; // check mbr, check ref point method, etc
			total_effort += rcell->points; // point in polygon, discard L str tree check O(1) or O(rcell->points), is Theta(x) better?

			// even if the estimate is equal zero, there can be objects that 
			// intersect each other, because there are an intersection between
			// the mbr of the lcell and rcell 
			if (intersections < 0.00001)
				intersections = 0.00001;

			resultcell->cardin += intersections;
			resultcell->objcount += intersections; //MAX(1.0,intersections);

		}

		total_effort += lcell->prep_cost;

		// estimate the prep_cost on the result cell
		resultcell->prep_cost = (lcell->prep_cost / lcell->objcount) * resultcell->objcount;
		// estimate the number of points for the result cell: average of objects points
		resultcell->points = (lcell->points / lcell->objcount) * resultcell->objcount;

		g_list_free(dr_cells);

		// store data to optimization and running
		if (cur_right_cells_int > 0) {
			opt_data[opt_atu].xl = lcell->x;
			opt_data[opt_atu].yl = lcell->y;
			opt_data[opt_atu].lcell = lcell;
			opt_data[opt_atu].pnts = total_effort;
			opt_data[opt_atu].comm = g_memdup(&total_io[0], sizeof total_io);
			opt_data[opt_atu].rcells_size = cur_right_cells_int;
			opt_data[opt_atu].rcells = g_memdup(&right_cells_int[0], sizeof(right_opt_data)*cur_right_cells_int);
			opt_atu++;
			if (opt_atu == opt_size) {
				opt_size *= 2;
				opt_data = g_renew(optimization_data_s, opt_data, opt_size);
			}
		}
	}
	g_list_free(dl_cells);

	char *qname = getenv("MW_QNAME");
	char *method = getenv("MW_OPTMETHOD");
	if (!method)
		method = "LP";

	reset_opt_data_copies(&result, opt_data, opt_atu);

	//persist_opt_data(&result, dl->metadata.mbr, opt_data, opt_atu, servers-1, level, agg_server, f);

	printf("Starting optimization for %s, %d pairs.\n", method, opt_atu);
	struct timespec cs, cf;
	clock_gettime(CLOCK_REALTIME, &cs);

	if (strncmp(method, "BS", 2) == 0) {
		bs_optimize_hr(&result, servers, opt_data, opt_atu, agg_server, f);
	}
	else if (strncmp(method, "LR", 2) == 0) {
		method = "GR";
		//lp_optimize_hr(&result, servers-1, opt_data, opt_atu, agg_server, f, NULL);
		bs_optimize_hr(&result, servers, opt_data, opt_atu, agg_server, f);

		method = "LR";
		reset_opt_data_copies(&result, opt_data, opt_atu);
		lagrange_sm_optimize_hr(&result, servers-1, opt_data, opt_atu, agg_server, f, NULL);
	}
	else if (strncmp(method, "LP", 2) == 0) {
		method = "LP";
		lp_optimize_hr(&result, servers-1, opt_data, opt_atu, agg_server, f, NULL);
	}
	else {
		printf("Optimization method not know %s\n", method);
		assert(0);
	}

	clock_gettime(CLOCK_REALTIME, &cf);
	double runtime = runtime_diff_ms(&cs, &cf);

	totalize_estimate(&result, estimate, servers-1, opt_data, opt_atu);

	if (verbose) {
		printf("Optimization runtime %'10.2f ms\n", runtime);
		printf("\nBased on %s method:\n", method);
		multiway_print_estimate("server", estimate, servers-1);
	}

	double totalcomm, mksp, stdevmksp;
	multiway_totalize_estimate(estimate, servers-1, NULL, &totalcomm, 
		&mksp, NULL, &stdevmksp, NULL, NULL);
	// write results to file
	if (fresults) {
		fprintf(fresults, "%s\t%d\t%d\t%12.0f\t%12.0f\t%12.0f\t%10.2f\n", method, opt_atu, servers-1, 
			mksp, totalcomm, stdevmksp, runtime);
	}

	if (opt_data_level != NULL) {
		opt_data_level->opt_data = opt_data;
		opt_data_level->opt_data_size = opt_atu;
		opt_data_level->dl = dl;
		opt_data_level->dr = dr;
		opt_data_level->result = result;
	}
	else {
		// if caller doen't need opt_data, free its memory
		free_opt_data(opt_data, opt_atu);
	}

	g_free(right_cells_int);
	return result;
}

void free_opt_data(optimization_data_s *opt_data, int opt_size) {
	// free optimization struct
	for(int i = 0; i < opt_size; i++) {
		g_free(opt_data[i].comm);
		g_free(opt_data[i].rcells);
	}
	g_free(opt_data);
}


int compare_right_opt_data(const void* x, const void*y) {
	right_opt_data *xc = *(right_opt_data**)x;
	right_opt_data *yc = *(right_opt_data**)y;
	if (xc->xr > yc->xr) return 1;
	if (xc->xr < yc->xr) return -1;
	if (xc->xr == yc->xr) {
		if (xc->yr < yc->yr) return 1;
		else if (xc->yr > yc->yr) return -1;
	}
	return 0; 	
}

void totalize_estimate(dataset_histogram *hr, multiway_histogram_estimate *estimate, 
	int servers, optimization_data_s *opt_data, int pairs) {

	// clear the estimate structure to refill with the optimized schedule
	memset(estimate, 0, sizeof(multiway_histogram_estimate)*(servers+1));

	typedef struct {
		int key;
		uint64_t place;
		UT_hash_handle hh;
	} used_rcells;
	used_rcells *rcells = NULL;

	for(int i = 0; i < pairs; i++) {
		// to_pnts
		histogram_cell *resultcell = hr->get_cell(hr, opt_data[i].xl, opt_data[i].yl);
		estimate[resultcell->place].to_pnts += opt_data[i].pnts;

		// lcell io_pnts
		if (opt_data[i].lcell->place != resultcell->place)
			estimate[resultcell->place].io_pnts += opt_data[i].lcell->points;

		// rcell's
		for(int c = 0; c < opt_data[i].rcells_size; c++) {
			if (opt_data[i].rcells[c].cell->place != resultcell->place) {
				right_opt_data *rcell = &opt_data[i].rcells[c];

				int rid = rcell->xr + (rcell->yr<<16);
				used_rcells *r;
				HASH_FIND_INT(rcells, &rid, r);
				if (r == NULL) {
					r = g_new(used_rcells, 1);
					r->key = rid;
					r->place = 0;
					SET_IN_PLACE(r->place, resultcell->place);
					HASH_ADD_INT(rcells, key, r);
					estimate[resultcell->place].io_pnts += rcell->cell->points;
				} else if (!IS_IN_PLACE(r->place, resultcell->place)) {
					SET_IN_PLACE(r->place, resultcell->place);
					estimate[resultcell->place].io_pnts += rcell->cell->points;
				}
			}
		}
	}

	used_rcells *current, *tmp;
	HASH_ITER(hh, rcells, current, tmp) {
	    HASH_DEL(rcells, current);
    	g_free(current);
	}
};

void reset_opt_data_copies(dataset_histogram *hr, optimization_data_s *opt_data, int opt_atu) {
	// reset previous copies
	for(int cell = 0; cell < opt_atu; cell++) {
		histogram_cell *lc = opt_data[cell].lcell;
		lc->copies = 0;
		SET_IN_PLACE(lc->copies, lc->place);

		for(int c = 0; c < opt_data[cell].rcells_size; c++) {
			histogram_cell *rc = opt_data[cell].rcells[c].cell;
			rc->copies = 0;
			SET_IN_PLACE(rc->copies, rc->place);
		}
	}
}

typedef struct {
	int x;
	int y;
	float *cost;
} persistij;

typedef struct {
	int i;
	int j;
} pair;

int exist_persistij(int x, int y, persistij pij[], int t) {
	for(int i = t-1; i >= 0; i--) {
		if (pij[i].x == x && pij[i].y == y)
			return i;
	}
	return -1;
}

void persist_opt_data(dataset_histogram *hr, Envelope lmbr, optimization_data_s *opt_data, int opt_atu, int servers, int step, multiway_histogram_estimate *agg_server, double f) {

	char fname[100];
	char *qname = getenv("MW_QNAME");
	sprintf(fname, "opt_data_%s_%d_%d.bin", qname, step+1, servers);
	FILE *df = fopen(fname, "w");

/*	fwrite(&opt_atu, sizeof(int), 1, df);
	fwrite(&servers, sizeof(int), 1, df);

	fwrite(hr, sizeof(dataset_histogram), 1, df);

	dataset ds;
	ds.metadata.hist = *hr;
	ds.metadata.mbr = lmbr;
	int size;
	dataset_histogram_persist *data = hr->get_data(&ds, &size);
	fwrite(&size, sizeof(int), 1, df);
	fwrite(data, size, 1, df);

	for(int i = 0; i < opt_atu; i++) {
		fwrite(&opt_data[i], sizeof(optimization_data_s), 1, df);
		fwrite(opt_data[i].lcell, sizeof(histogram_cell), 1, df);
		fwrite(opt_data[i].comm, sizeof(double), servers+1, df);
		for(int j = 0; j < opt_data[i].rcells_size; j++) {
			fwrite(&opt_data[i].rcells[j], sizeof(right_opt_data), 1, df);
			fwrite(opt_data[i].rcells[j].cell, sizeof(histogram_cell), 1, df);
		}
	}

	fwrite(agg_server, sizeof(multiway_histogram_estimate), servers+1, df);
	*/
	int p = opt_atu;
	int q = 0;
	persistij *is = malloc(sizeof(persistij) * opt_atu);

	int m = servers;
	int n = 0;
	for(int i = 0; i < opt_atu; i++)
		n += opt_data[i].rcells_size;

	persistij *js = malloc(sizeof(persistij) * n);

	int patu = 0;
	float wij[n];
	pair pairs[n];
	
	for(int i = 0; i < opt_atu; i++) {
		is[i].x = opt_data[i].xl;
		is[i].y = opt_data[i].yl;
		is[i].cost = malloc(sizeof(float) * servers);
		for(int k = 0; k < servers; k++) {
			if (!IS_IN_PLACE(k+1, opt_data[i].lcell->place))
				is[i].cost[k] = opt_data[i].lcell->points;
			else
				is[i].cost[k] = 0;
		}

		for(int j = 0; j < opt_data[i].rcells_size; j++) {
			int jid = exist_persistij(opt_data[i].rcells[j].xr, opt_data[i].rcells[j].yr, js, q);
			if (jid == -1) {
				assert(q < n);
				js[q].x = opt_data[i].rcells[j].xr;
				js[q].y = opt_data[i].rcells[j].yr;
				js[q].cost = malloc(sizeof(float) * servers);				
				for(int k = 0; k < servers; k++) {
					if (!IS_IN_PLACE(k+1, opt_data[i].rcells[j].cell->place))
						js[q].cost[k] = opt_data[i].rcells[j].cell->points;
					else
						js[q].cost[k] = 0;
				}
				jid = q;
				q++;
			}

			// according to total_effort above

			// I had to put costs only on the first pairs containing i 
			// to maintain compatibility with the way that instances where generated for the thesis.
			// In the thesis, there is only one pair for each lcell, and the
			// network cost is computed separately. The lcell is transfered
			// for only one machine. Make sure to enable ALL_I_IN_SAME_SERVER for it to work on fmmodel project
/*			if (j == 0)
				wij[patu] = opt_data[i].pnts;
			else
				wij[patu] = 0.0;*/

			wij[patu] = 
				opt_data[i].lcell->prep_cost +
				opt_data[i].rcells[j].cell->points +
				opt_data[i].lcell->objcount * opt_data[i].rcells[j].cell->objcount;
			pairs[patu].i = i;
			pairs[patu].j = jid;
			patu++;
		}			
	}

	fprintf(df, "n=%d m=%d p=%d q=%d f=%f\n", n, m, p, q, f);

	// pairs
	for(int ij = 0; ij < n; ij++) {
		fprintf(df, "(%d,%d)\t", pairs[ij].i, pairs[ij].j);
	}
	fprintf(df, "\n");

	// wij
	for(int ij = 0; ij < n; ij++) {
		fprintf(df, "%f\t", wij[ij]);
	}
	fprintf(df, "\n");

	// ca
	for(int k = 0; k < m; k++) {
		for(int i = 0; i < p; i++) {
			fprintf(df, "%f\t", is[i].cost[k]);
		}
		fprintf(df, "\n");
	}

	// cb
	for(int k = 0; k < m; k++) {
		for(int j = 0; j < q; j++) {
			fprintf(df, "%f\t", js[j].cost[k]);
		}
		fprintf(df, "\n");
	}

	// free is and js
	for(int i = 0; i < opt_atu; i++)
		free(is[i].cost);
	free(is);

	for(int i = 0; i < q; i++)
		free(js[i].cost);
	free(js);

	fclose(df);
	printf("Done persisting opt data to %s\n", fname);

}

