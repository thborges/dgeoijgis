
#include "dataset.h"
#include "histcommon.h"

dataset_histogram_persist *dataset_get_histogram(dataset *dh, int *size) {
	return dh->metadata.hist.get_data(dh, size);
}

void dataset_set_histogram(dataset *ds, dataset_histogram_persist *hp) {

	ds->metadata.hist.htype = hp->htype;

	switch (ds->metadata.hist.htype) {
		case HIST_GRID:
			histogram_set_functions_grid(&ds->metadata.hist);
			break;

		default:
			fprintf(stderr, "Invalid histogram type %d\n", hp->htype);
			break;
	}

	ds->metadata.hist.set_data(ds, hp);
}

double OLD_AVG_LENGTH_X(histogram_cell *cell) {
	double total = 0;
	int qtd = 0;
	for(int bx = 0; bx < AVGL_HISTO_SIZE; bx++) {
		total += cell->len[bx].avg_x * cell->len[bx].qtd_x;
		qtd += cell->len[bx].qtd_x;
	}
	return total / MAX(1,qtd);
}

double OLD_AVG_LENGTH_Y(histogram_cell *cell) {
	double total = 0;
	int qtd = 0;
	for(int bx = 0; bx < AVGL_HISTO_SIZE; bx++) {
		for(int by = 0; by < AVGL_HISTO_SIZE; by++) {
			total += cell->len[bx].avg_y[by] * cell->len[bx].qtd_y[by];
			qtd += cell->len[bx].qtd_y[by];
		}
	}
	return total / MAX(1,qtd);
}

