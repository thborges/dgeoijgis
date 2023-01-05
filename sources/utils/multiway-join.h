
#ifndef MULTIWAY_JOIN
#define MULTIWAY_JOIN

#ifdef __cplusplus
extern "C" {
#endif

#include "glibwrap.h"
#include "dataset.h"

enum JoinAlgorithm {
	join_none,
	join_inl,
	join_rj,
	join_hj,
	join_p_hj,
	join_p_rj,
	join_p_inl,
};

extern const char *JoinAlgorithmNames[];

typedef struct {
	GList *tuple;
} multiway_output;

typedef struct {
	int xr;
	int yr;
	histogram_cell *cell;
} right_opt_data;

typedef struct {
	int xl;
	int yl;
	histogram_cell *lcell;
	double pnts;
	double *comm;
	size_t rcells_size;
	right_opt_data *rcells;
} optimization_data_s;

typedef struct {
	int opt_data_size;
	optimization_data_s *opt_data;
	dataset *dl;
	dataset *dr;
	dataset_histogram result;
} optimization_data_level;

typedef struct {
	union {
		void *rtree;
		dataset *dataset;
	};
	enum JoinAlgorithm algorithm;
	enum JoinPredicateCheck pcheck;
	char *name;
} multiway_input_chain;

int sort_optdata_decreasing(const void *x, const void *y);

void multiway_join_chain(GList *input);

dataset_histogram do_join_over_hist(dataset *dl, dataset *dr, unsigned char level,
	multiway_histogram_estimate *estimate, multiway_histogram_estimate *agg_server,
	optimization_data_level *opt_data_level, FILE *fresults);


void free_opt_data(optimization_data_s *opt_data, int opt_size);

#ifdef __cplusplus
}
#endif

#endif

