/*
 * histcommon.h
 *
 *  Created on: 2016-10-30
 *      Author: 
 */

#ifndef HISTCOMMON_H
#define HISTCOMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#include <glibwrap.h>
#include "ogrext.h"
#include "joincommon.h"

#define SET_IN_PLACE(var, place) (var |= ((uint64_t)1<<(uint64_t)(place-1)))
#define IS_IN_PLACE(var, place) (((var) >> (place-1)) & 1)

#define HIST_EPSILON 1e-100

#define AVGL_HISTO_SIZE 4

// buckets equally divided
#define AVGL_HISTO_DIV 3.999999
#define AVGLENGTH_FIND_BUCKET(olength,dim_length) ((olength) / ((dim_length) / AVGL_HISTO_DIV))

#define OLD_AVG_LENGTH(len, avg) ((len[0]*avg[0]+len[1]*avg[1]+len[2]*avg[2]+len[3]*avg[3])/MAX(1,(len[0]+len[1]+len[2]+len[3])))

typedef struct dataset_head_ dataset;

enum HistogramHashMethod { 
	HHASH_MBRCENTER, 
	HHASH_CENTROID, 
	HHASH_AREAFRAC, 
};

enum HistogramType {
	HIST_GRID,
};

typedef struct {
	enum HistogramType htype;
	Envelope mbr;
	char extra_data[1];
} dataset_histogram_persist;

typedef struct {
	unsigned short qtd_x;
	float avg_x;
	unsigned short qtd_y[AVGL_HISTO_SIZE];
	float avg_y[AVGL_HISTO_SIZE];
} avglength2d;

typedef struct {
	unsigned short x;
	unsigned short y;
	double cardin;
	double points;
	unsigned place;
	uint64_t copies;
	double objcount;
	float prep_cost;
	float areasum;
	Envelope usedarea;
	avglength2d len[AVGL_HISTO_SIZE];
	void *extra_data;
} histogram_cell;

double OLD_AVG_LENGTH_X(histogram_cell *cell);
double OLD_AVG_LENGTH_Y(histogram_cell *cell);

typedef struct dataset_histogram dataset_histogram;

typedef	struct dataset_histogram {
	enum HistogramType htype;
	OGRwkbGeometryType geom_type;
	void *extra_data;

	int (*free)(dataset *ds);
	void (*distribute)(dataset *ds);
	GList* (*intersects)(dataset *ds, Envelope ev);
	void (*clone_histogram)(dataset *dh, dataset_histogram *result);
	Envelope (*get_cell_envelope)(dataset_histogram *dh, histogram_cell *cell);
	histogram_cell *(*get_cell)(dataset_histogram *dh, unsigned short x, unsigned short y);
	histogram_cell *(*get_reference_cell)(dataset *ds, double x, double y);

	dataset_histogram_persist *(*get_data)(dataset *dh, int *size);
	void (*set_data)(dataset *dh, dataset_histogram_persist *data);
} dataset_histogram;

void histogram_generate_grid(dataset *ds, enum HistogramHashMethod hm, enum JoinPredicateCheck pcheck);
void histogram_generate_minskew(dataset *ds, enum HistogramHashMethod hm, enum JoinPredicateCheck pcheck);

void histogram_set_functions_grid(dataset_histogram *dh);

dataset_histogram_persist *dataset_get_histogram(dataset *dh, int *size);
void dataset_set_histogram(dataset *dh, dataset_histogram_persist *hp);

#ifdef __cplusplus
}
#endif

#endif

