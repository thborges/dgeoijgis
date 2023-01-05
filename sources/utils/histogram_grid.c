/*
 * histogram_grid.h
 *
 *  Created on: 2016-10-30
 *      Author: 
 */

#include <assert.h>
#include <float.h>

#include "dataset.h"
#include "geosext.h"
#include "histcommon.h"
#include "histogram_grid.h"
#include "glibwrap.h"

void histogram_build_metadata(dataset *ds, enum JoinPredicateCheck pcheck);
void histogram_generate_hw(dataset *ds, double x, double y, enum HistogramHashMethod hm, enum JoinPredicateCheck pcheck);
void histogram_generate_fix(dataset *ds, int fsizex, int fsizey, enum HistogramHashMethod hm, enum JoinPredicateCheck pcheck);
void histogram_distribute(dataset *ds);
int histogram_grid_free(dataset *ds); 
GList* histogram_intersects(dataset *ds, Envelope ev);
void histogram_clonemetadata(dataset *ds, dataset_histogram *result);
Envelope histogram_get_cell_envelope(dataset_histogram *dh, histogram_cell *cell);
histogram_cell *histogram_get_cell(dataset_histogram *dh, unsigned short x, unsigned short y);
histogram_cell *histogram_get_reference_cell(dataset *ds, double x, double y);

dataset_histogram_persist *histogram_get_data_grid(dataset *ds, int *size);
void histogram_set_data_grid(dataset *ds, dataset_histogram_persist *data);

void histogram_set_functions_grid(dataset_histogram *dh) {
	dh->htype = HIST_GRID;
	dh->distribute = histogram_distribute;
	dh->free = histogram_grid_free;
	dh->intersects = histogram_intersects;
	dh->clone_histogram = histogram_clonemetadata;
	dh->get_cell_envelope = histogram_get_cell_envelope;
	dh->get_cell = histogram_get_cell;
	dh->get_reference_cell = histogram_get_reference_cell;
	dh->get_data = histogram_get_data_grid;
	dh->set_data = histogram_set_data_grid;
}
 
void histogram_generate_grid(dataset *ds, enum HistogramHashMethod hm, enum JoinPredicateCheck pcheck) {

	histogram_set_functions_grid(&ds->metadata.hist);
	histogram_build_metadata(ds, pcheck);

	histogram_generate_hw(ds, 
		4*(ds->metadata.x_average + dataset_meta_stddev(ds->metadata, x)), 
		4*(ds->metadata.y_average + dataset_meta_stddev(ds->metadata, y)),
		hm, pcheck);

}

void histogram_alloc(dataset_histogram *dh, int xqtd, int yqtd) {
	assert(xqtd > 0 && yqtd > 0 && "X and Y must be greater than zero.");
	grid_histogram_data *ghd = g_new0(grid_histogram_data, 1);
	dh->extra_data = ghd;
	ghd->xqtd = xqtd;
	ghd->yqtd = yqtd;
	ghd->xtics = g_new(double, xqtd+1);
	ghd->ytics = g_new(double, yqtd+1);
	ghd->hcells = g_new0(histogram_cell, xqtd*yqtd);

	for(int x = 0; x < xqtd; x++) {
		for(int y = 0; y < yqtd; y++) {
			histogram_cell *cell = GET_GRID_HISTOGRAM_CELL(ghd, x, y);
			cell->x = x;
			cell->y = y;
			cell->usedarea.MinX = cell->usedarea.MinY = DBL_MAX;
			cell->usedarea.MaxX = cell->usedarea.MaxY = -DBL_MAX;
		}
	}
}

void histogram_clonemetadata(dataset *ds, dataset_histogram *result) {
	
	grid_histogram_data *ghd = (grid_histogram_data*)ds->metadata.hist.extra_data;
	histogram_alloc(result, ghd->xqtd, ghd->yqtd);
	histogram_set_functions_grid(result);

	grid_histogram_data *ghd_new = (grid_histogram_data*)result->extra_data;

	memcpy(ghd_new->xtics, ghd->xtics, sizeof(double)*(ghd->xqtd+1));
	memcpy(ghd_new->ytics, ghd->ytics, sizeof(double)*(ghd->yqtd+1));
	//result.mbr = hl->mbr;
	ghd_new->xsize = ghd->xsize;
	ghd_new->ysize = ghd->ysize;
}

dataset_histogram histogram_join_io(dataset *dr, dataset *ds, enum JoinPredicateCheck next_pcheck, multiway_histogram_estimate *estimate);

inline __attribute__((always_inline))
void fill_hist_cell_centroid(dataset_leaf *l, dataset *ds, dataset_histogram *dh) {

	GEOSGeometryH geo = dataset_get_leaf_geo(ds, l);
	GEOSGeometryH centroid = GEOSGetCentroid(geo);

	double x, y;
	GEOSGeomGetX(centroid, &x);
	GEOSGeomGetY(centroid, &y);

	grid_histogram_data *ghd = (grid_histogram_data*)dh->extra_data;
	int xp = (x - ds->metadata.mbr.MinX) / ghd->xsize;
	int yp = (y - ds->metadata.mbr.MinY) / ghd->ysize;
	histogram_cell *cell = &ghd->hcells[xp * ghd->yqtd +yp];
	cell->cardin++;
	cell->points += l->points;

	GEOSGeom_destroy(centroid);
	if (l->gid != -1) // free because of the call to dataset_get_leaf_geo
		GEOSGeom_destroy(geo);
}

void get_xini_xfim(dataset *ds, Envelope query, 
	int *xini, int *xfim, int *yini, int *yfim) {

	grid_histogram_data *ghd = (grid_histogram_data*)ds->metadata.hist.extra_data;

	// prevent values of x and y out of histogram bounds
	if (!ENVELOPE_INTERSECTS(query, ds->metadata.mbr)) {
		*xini = *yini = 0;
		*xfim = *yfim = -1;
	} else {
		// prevent values of x and y out of histogram bounds
		query = EnvelopeIntersection2(query, ds->metadata.mbr);

		*xini = (query.MinX - ds->metadata.mbr.MinX) / ghd->xsize;
		*xfim = (query.MaxX - ds->metadata.mbr.MinX) / ghd->xsize;
		*yini = (query.MinY - ds->metadata.mbr.MinY) / ghd->ysize;
		*yfim = (query.MaxY - ds->metadata.mbr.MinY) / ghd->ysize;

		if (*xfim == ghd->xqtd) (*xfim)--;
		if (*yfim == ghd->yqtd) (*yfim)--;
   
		while (ghd->xtics[*xini]     > query.MinX) (*xini)--;
		while (ghd->xtics[(*xfim)+1] < query.MaxX) (*xfim)++;
		while (ghd->ytics[*yini]     > query.MinY) (*yini)--;
		while (ghd->ytics[(*yfim)+1] < query.MaxY) (*yfim)++;

		assert(*xini >= 0 && *xfim < ghd->xqtd && "x is out of histogram bounds.");
		assert(*yini >= 0 && *yfim < ghd->yqtd && "y is out of histogram bounds.");
	}
}

inline __attribute__((always_inline))
void fill_hist_cell_mbr_center(dataset_leaf *l, dataset *ds, dataset_histogram *dh) {
	// using mbr center
	double x = l->mbr.MinX + (l->mbr.MaxX - l->mbr.MinX) / 2.0;
	double y = l->mbr.MinY + (l->mbr.MaxY - l->mbr.MinY) / 2.0;

	grid_histogram_data *ghd = (grid_histogram_data*)dh->extra_data;
	int xp = (x - ds->metadata.mbr.MinX) / ghd->xsize;
	int yp = (y - ds->metadata.mbr.MinY) / ghd->ysize;

	if (xp == ghd->xqtd) xp--;
	if (yp == ghd->yqtd) yp--;

	histogram_cell *cell = &ghd->hcells[xp*ghd->yqtd +yp];
	cell->cardin += 1.0;
	cell->objcount += 1.0;
	cell->points += l->points;
	cell->prep_cost += l->points * log10(l->points); // cost of making a STR-Tree, m=10, n log_10 n

	// average length, online average calculation
	double delta_x = MIN(l->mbr.MaxX - l->mbr.MinX, ghd->xsize);
	double delta_y = MIN(l->mbr.MaxY - l->mbr.MinY, ghd->ysize);
	int bucketx = AVGLENGTH_FIND_BUCKET(delta_x, ghd->xsize);
	int buckety = AVGLENGTH_FIND_BUCKET(delta_y, ghd->ysize);
	cell->len[bucketx].qtd_x += 1;
	cell->len[bucketx].avg_x += 
		(delta_x - cell->len[bucketx].avg_x) / cell->len[bucketx].qtd_x;
	cell->len[bucketx].qtd_y[buckety] += 1;
	cell->len[bucketx].avg_y[buckety] += 
		(delta_y - cell->len[bucketx].avg_y[buckety]) / cell->len[bucketx].qtd_y[buckety];
}

double hash_envelope_area_fraction(dataset *ds, Envelope ev, double objarea, double points) {

	int xini, xfim, yini, yfim;
	get_xini_xfim(ds, ev, &xini, &xfim, &yini, &yfim);	

	grid_histogram_data *ghd = (grid_histogram_data*)ds->metadata.hist.extra_data;

	double sum_fraction = 0;
	for(int x = xini; x <= xfim; x++) {
		Envelope rs;
		rs.MinX = ghd->xtics[x];
		rs.MaxX = ghd->xtics[x+1];

		for(int y = yini; y <= yfim; y++) {
			rs.MinY = ghd->ytics[y];
			rs.MaxY = ghd->ytics[y+1];

			Envelope inters = EnvelopeIntersection(ev, rs);
			double intarea = ENVELOPE_AREA(inters);
			double fraction;
			if (intarea <= 0.0) { // parallel to one axis
				bool parallel_y = (ev.MaxX - ev.MinX < 1e-30);
				bool parallel_x = (ev.MaxY - ev.MinY < 1e-30);
				if (parallel_x && parallel_y) // point obj
					fraction = 1;
				else if (parallel_x)
					// the part of ev inside rs / the length of rs in X
					fraction = (MIN(rs.MaxX, ev.MaxX) - MAX(rs.MinX, ev.MinX)) / (ev.MaxX - ev.MinX);
				else
					fraction = (MIN(rs.MaxY, ev.MaxY) - MAX(rs.MinY, ev.MinY)) / (ev.MaxY - ev.MinY);
			}
			else {
				fraction = intarea / objarea;
			}
			sum_fraction += fraction;

			histogram_cell *cell = GET_GRID_HISTOGRAM_CELL(ghd, x, y);
			cell->cardin += fraction;
			cell->objcount += 1.0;
			cell->points += points; //object is replicated
			cell->prep_cost += points * log10(points); // cost of making a STR-Tree, m=10, n log_10 n

			// average length, online average calculation
			double delta_x = (inters.MaxX - inters.MinX);
			double delta_y = (inters.MaxY - inters.MinY);
			int bucketx = AVGLENGTH_FIND_BUCKET(delta_x, ghd->xsize);
			int buckety = AVGLENGTH_FIND_BUCKET(delta_y, ghd->ysize);
			cell->len[bucketx].qtd_x += 1;
			cell->len[bucketx].avg_x += 
				(delta_x - cell->len[bucketx].avg_x) / cell->len[bucketx].qtd_x;
			cell->len[bucketx].qtd_y[buckety] += 1;
			cell->len[bucketx].avg_y[buckety] += 
				(delta_y - cell->len[bucketx].avg_y[buckety]) / cell->len[bucketx].qtd_y[buckety];
		}
	}
	return sum_fraction;
}

inline __attribute__((always_inline))
void fill_hist_cell_area_fraction(dataset_leaf *l, dataset *ds, dataset_histogram *dh) {
	// proportional to cover area
	double objarea = ENVELOPE_AREA(l->mbr);
	hash_envelope_area_fraction(ds, l->mbr, objarea, l->points);
}

void envelope_update(Envelope *e, double X, double Y) {
	e->MinX = MIN(e->MinX, X);
	e->MinY = MIN(e->MinY, Y);
	e->MaxX = MAX(e->MaxX, X);
	e->MaxY = MAX(e->MaxY, Y);
}

void histogram_generate_cells_fix(dataset *ds, double psizex, double psizey, enum HistogramHashMethod hm, enum JoinPredicateCheck pcheck) {

	dataset_histogram *dh = &ds->metadata.hist;
	grid_histogram_data *ghd = (grid_histogram_data*)dh->extra_data;
	ghd->xsize = psizex;
	ghd->ysize = psizey;
	printf("Generating histogram of size: %d x %d\n", ghd->xqtd, ghd->yqtd);

	// X
	double xini = ds->metadata.mbr.MinX;
	for(int i = 0; i < ghd->xqtd; i++)
		ghd->xtics[i] = xini + (psizex * i);
	ghd->xtics[ghd->xqtd] = ds->metadata.mbr.MaxX;

	// Y
	double yini = ds->metadata.mbr.MinY;
	for(int i = 0; i < ghd->yqtd; i++)
		ghd->ytics[i] = yini + (psizey * i);
	ghd->ytics[ghd->yqtd] = ds->metadata.mbr.MaxY;

	double total_area, split_area = 0;
	dataset_iter di;
	dataset_foreach(di, ds) {
		dataset_leaf *l = get_join_pair_leaf(di.item, pcheck);

		switch (hm) {
			case HHASH_CENTROID:
				fill_hist_cell_centroid(l, ds, dh);
				break;
	
			case HHASH_MBRCENTER: 
				fill_hist_cell_mbr_center(l, ds, dh);
				break;

			case HHASH_AREAFRAC:
				fill_hist_cell_area_fraction(l, ds, dh);
				break;

			default:
				printf("Histogram method not defined.\n");
		}
	}

	printf("**** total area %f, split area %f\n", total_area, split_area);
}

void histogram_generate_avg(dataset *ds, enum HistogramHashMethod hm, enum JoinPredicateCheck pcheck) {

	double rangex = ds->metadata.mbr.MaxX - ds->metadata.mbr.MinX;
	double rangey = ds->metadata.mbr.MaxY - ds->metadata.mbr.MinY;

	double psizex = ds->metadata.x_average;
	double psizey = ds->metadata.y_average;

	dataset_histogram *dh = &ds->metadata.hist;
	histogram_alloc(dh, ceil(rangex / psizex), ceil(rangey / psizey));

	histogram_generate_cells_fix(ds, psizex, psizey, hm, pcheck);

};

void histogram_generate_hw(dataset *ds, double x, double y, enum HistogramHashMethod hm, enum JoinPredicateCheck pcheck) {

	double rangex = ds->metadata.mbr.MaxX - ds->metadata.mbr.MinX;
	double rangey = ds->metadata.mbr.MaxY - ds->metadata.mbr.MinY;

	double psizex = x;
	double psizey = y;

	int qtd_x = ceil(rangex / psizex);
	int qtd_y = ceil(rangey / psizey);

	// at least 5x5
	qtd_x = MAX(5, qtd_x);
	qtd_y = MAX(5, qtd_y);
	
	// at most 2MB per histogram
	const int upper_bound_cell_number = 2*1024*1024 / 72.0;//sizeof(histogram_cell);
	double adjust = sqrt((qtd_x * qtd_y) / (double)upper_bound_cell_number);
	if (adjust > 1.0) {
		qtd_x = ceil(rangex / (psizex * adjust));
		qtd_y = ceil(rangey / (psizey * adjust));
	}

	// recompute cell length in each dimension
	psizex = rangex / qtd_x;
	psizey = rangey / qtd_y;

	dataset_histogram *dh = &ds->metadata.hist;
	histogram_alloc(dh, qtd_x, qtd_y);

	histogram_generate_cells_fix(ds, psizex, psizey, hm, pcheck);

};

void histogram_generate_fix(dataset *ds, int fsizex, int fsizey, enum HistogramHashMethod hm, enum JoinPredicateCheck pcheck) {

	double rangex = ds->metadata.mbr.MaxX - ds->metadata.mbr.MinX;
	double rangey = ds->metadata.mbr.MaxY - ds->metadata.mbr.MinY;

	double psizex = rangex / fsizex;
	double psizey = rangey / fsizey;

	dataset_histogram *dh = &ds->metadata.hist;
	histogram_alloc(dh, fsizex, fsizey);

	histogram_generate_cells_fix(ds, psizex, psizey, hm, pcheck);
	
};


void histogram_build_metadata(dataset *ds, enum JoinPredicateCheck pcheck) {

	char first = 1;
	dataset_iter di;
    dataset_foreach(di, ds) {
		dataset_leaf *leaf = get_join_pair_leaf(di.item, pcheck);
		
		// metadata: dataset extent
		double new_x = leaf->mbr.MaxX - leaf->mbr.MinX;
		double new_y = leaf->mbr.MaxY - leaf->mbr.MinY;
		//printf("%20.10f\n", new_x);
		if (first) {
			first = 0;
			ds->metadata.mbr = leaf->mbr;
			ds->metadata.x_average = new_x;
			ds->metadata.y_average = new_y;
			ds->metadata.x_psa = ds->metadata.y_psa = 0.0;
		}
		else {
			ENVELOPE_MERGE(ds->metadata.mbr, leaf->mbr);

			double oldmx = ds->metadata.x_average;
			double oldmy = ds->metadata.y_average;
			ds->metadata.x_average += (new_x - ds->metadata.x_average) / ds->metadata.count;
			ds->metadata.y_average += (new_y - ds->metadata.y_average) / ds->metadata.count;
			ds->metadata.x_psa += (new_x - oldmx) * (new_x - ds->metadata.x_average);
			ds->metadata.y_psa += (new_y - oldmy) * (new_y - ds->metadata.y_average);
		}
	}
}

void histogram_distribute_roundrobin(dataset *ds) {
	if (ds->metadata.servers <= 0)
		return;

	//printf("Distributing histogram for %d servers using round-robin.\n", ds->metadata.servers);

	dataset_histogram *hist = &ds->metadata.hist;
	grid_histogram_data *ghd = (grid_histogram_data*)hist->extra_data;

	int current_server = 0;

	for(int x = 0; x < ghd->xqtd; x++) {
		for(int y = 0; y < ghd->yqtd; y++) {
			histogram_cell *cell = GET_GRID_HISTOGRAM_CELL(ghd, x, y);
			if (cell->objcount > 0.0) {
				cell->copies = 0;
				cell->place = current_server+1;
				SET_IN_PLACE(cell->copies, cell->place);
				current_server = (current_server+1) % ds->metadata.servers;
			}
		}
	}
}

void histogram_distribute(dataset *ds) {
	histogram_distribute_roundrobin(ds);
}

int histogram_grid_data_free(grid_histogram_data* ghd) {
	g_free(ghd->xtics);
	g_free(ghd->ytics);
	g_free(ghd->hcells);
	int xsize = ghd->xsize;
	int ysize = ghd->ysize;
	g_free(ghd);

    return
        sizeof(double) * (xsize + ysize) +
        sizeof(histogram_cell) * xsize * ysize;
}

int histogram_grid_free(dataset *ds) {
	grid_histogram_data *ghd = (grid_histogram_data*)ds->metadata.hist.extra_data;
	int s = histogram_grid_data_free(ghd);
	ds->metadata.hist.extra_data = NULL;
	return s;
}


GList* histogram_intersects(dataset *ds, Envelope ev) {

	int xini, xfim, yini, yfim;
	get_xini_xfim(ds, ev, &xini, &xfim, &yini, &yfim);	

	grid_histogram_data *ghd = (grid_histogram_data*)ds->metadata.hist.extra_data;

	GList *result = NULL;
	for(int x = xini; x <= xfim; x++) {
		for(int y = yini; y <= yfim; y++) {
			histogram_cell *cell = GET_GRID_HISTOGRAM_CELL(ghd, x, y);
			if (cell->objcount > 0.0) {
				result = g_list_prepend(result, cell);
			}
		}
	}
	return result;
}

Envelope histogram_get_cell_envelope(dataset_histogram *dh, histogram_cell *cell) {
	
	grid_histogram_data *ghd = (grid_histogram_data*)dh->extra_data;
	Envelope el;
	el.MinX = ghd->xtics[cell->x];
	el.MaxX = ghd->xtics[cell->x+1];
	el.MinY = ghd->ytics[cell->y];
	el.MaxY = ghd->ytics[cell->y+1];
	return el;
}

histogram_cell *histogram_get_cell(dataset_histogram *dh, unsigned short x, unsigned short y) {
	grid_histogram_data *ghd = (grid_histogram_data*)dh->extra_data;
	return GET_GRID_HISTOGRAM_CELL(ghd, x, y);
}

histogram_cell *histogram_get_reference_cell(dataset *ds, double x, double y) {
	// find the cell at point (x,y)	
	grid_histogram_data *ghd = (grid_histogram_data*)ds->metadata.hist.extra_data;
	int idx_x = (x - ds->metadata.mbr.MinX) / ghd->xsize;
	int idx_y = (y - ds->metadata.mbr.MinY) / ghd->ysize;
	if (!(x >= ghd->xtics[idx_x] && x <= ghd->xtics[idx_x+1])) {
		printf("WARNING: idx_x %d x %.10f min %.10f xsize %.10f xtics %.10f %.10f\n",
			idx_x, x, ds->metadata.mbr.MinX, ghd->xsize,
			ghd->xtics[idx_x], ghd->xtics[idx_x+1]);
		//assert(0);
	}
	if (!(y >= ghd->ytics[idx_y] && y <= ghd->ytics[idx_y+1])) {
		printf("WARNING: idx_y %d y %.10f min %.10f ysize %.10f ytics %.10f %.10f\n",
			idx_y, y, ds->metadata.mbr.MinY, ghd->ysize,
			ghd->ytics[idx_y], ghd->ytics[idx_y+1]);
		//assert(0);
	}
	return GET_GRID_HISTOGRAM_CELL(ghd, idx_x, idx_y);
}

dataset_histogram_persist *histogram_get_data_grid(dataset *ds, int *size) {

	grid_histogram_data *ghd = (grid_histogram_data*)ds->metadata.hist.extra_data;

	*size = sizeof(dataset_histogram_persist) +
		sizeof(grid_histogram_data) +
		(ghd->xqtd+1) * sizeof(double) + //xtics
		(ghd->yqtd+1) * sizeof(double) + //ytics
		ghd->xqtd * ghd->yqtd * sizeof(histogram_cell); //hcells

	char *data = g_new(char, *size);
	dataset_histogram_persist *hp =	(dataset_histogram_persist *)&data[0];
	hp->htype = ds->metadata.hist.htype;
	hp->mbr = ds->metadata.mbr;

	int pos = 0;

	grid_histogram_data *ghd_new = (grid_histogram_data*)&hp->extra_data[pos];
	*ghd_new = *ghd;
	pos += sizeof(grid_histogram_data);

	double *xtics = (double*)&hp->extra_data[pos];
	memcpy(xtics, ghd->xtics, sizeof(double)*(ghd->xqtd+1));
	pos += sizeof(double) * (ghd->xqtd+1);

	double *ytics = (double*)&hp->extra_data[pos];
	memcpy(ytics, ghd->ytics, sizeof(double)*(ghd->yqtd+1));
	pos += sizeof(double) * (ghd->yqtd+1);

	histogram_cell *hcells = (histogram_cell*)&hp->extra_data[pos];
	memcpy(hcells, ghd->hcells, sizeof(histogram_cell) * ghd->yqtd * ghd->xqtd);

	return hp;
}

void histogram_set_data_grid(dataset *ds, dataset_histogram_persist *hp) {

	ds->metadata.mbr = hp->mbr;

	grid_histogram_data *ghd_orig = (grid_histogram_data*)&hp->extra_data[0];

	histogram_alloc(&ds->metadata.hist, ghd_orig->xqtd, ghd_orig->yqtd);

	grid_histogram_data *ghd = (grid_histogram_data*)ds->metadata.hist.extra_data;
	ghd->xsize = ghd_orig->xsize;
	ghd->ysize = ghd_orig->ysize;

	int pos = sizeof(grid_histogram_data);

	double *xtics = (double*)&hp->extra_data[pos];
	memcpy(ghd->xtics, xtics, sizeof(double)*(ghd_orig->xqtd+1));
	pos += sizeof(double) * (ghd_orig->xqtd+1);

	double *ytics = (double*)&hp->extra_data[pos];
	memcpy(ghd->ytics, ytics, sizeof(double)*(ghd_orig->yqtd+1));
	pos += sizeof(double) * (ghd_orig->yqtd+1);

	histogram_cell *hcells = (histogram_cell*)&hp->extra_data[pos];
	memcpy(ghd->hcells, hcells, sizeof(histogram_cell) * ghd_orig->yqtd * ghd_orig->xqtd);
}

