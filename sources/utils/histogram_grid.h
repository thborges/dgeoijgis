/*
 * histogram_grid.h
 *
 *  Created on: 2016-10-30
 *      Author: 
 */

#ifndef HISTOGRAM_GRID_H
#define HISTOGRAM_GRID_H

#define GET_GRID_HISTOGRAM_CELL(h, x, y) (&((h)->hcells[(x)*(h)->yqtd + (y)]))

typedef struct {
	int xqtd;
	int yqtd;
	double xsize;
	double ysize;
	double *xtics;
	double *ytics;
	histogram_cell *hcells;
} grid_histogram_data;

int histogram_grid_data_free(grid_histogram_data *ghd);

#endif

