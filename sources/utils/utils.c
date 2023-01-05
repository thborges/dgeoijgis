
#include "utils.h"
#include <float.h>
#include <unistd.h>
#include <signal.h>
#include <execinfo.h>

float maxs(size_t data[], size_t start, size_t n) {
	double max = data[start];
	for(int i = start+1; i<n; i++) {
		if (max < data[i])
			max = data[i];
	}
	return max;
}

double maxdbl(double data[], size_t start, size_t n) {
	double max = data[start];
	for(int i = start+1; i<n; i++) {
		if (max < data[i])
			max = data[i];
	}
	return max;
}

double mindbl(double data[], size_t start, size_t n) {
	double min = data[start];
	for(int i = start+1; i<n; i++) {
		if (min > data[i])
			min = data[i];
	}
	return min;
}


float stdev(size_t data[], size_t n) {
	return stdevs(data, 0, n);
}

float stdevs(size_t data[], size_t start, size_t n) {
	float mean=0.0, sum_deviation=0.0;
	int i;
	for(i=start; i<n; ++i)
		mean+=data[i];
	mean=mean/(n-start);
	for(i=start; i<n; ++i)
		sum_deviation += (data[i]-mean)*(data[i]-mean);
	return sqrt(sum_deviation/(n-start));
}

double getvecvalue(const void *vec, const int n) {
	return ((double*)vec)[n];
}

double stdevd(double data[], size_t start, size_t n) {
	return stdevd_ex(&data, start, n, getvecvalue);
}

double stdevd_ex(void *data, size_t start, size_t n, double (*getv)(const void *, const int n)) {
	double mean=0.0, sum_deviation=0.0;
	int i;
	for(i=start; i<n; ++i)
		mean+=getv(data, i);
	mean=mean/(n-start);
	for(i=start; i<n; ++i)
		sum_deviation += (getv(data, i)-mean)*(getv(data, i)-mean);
	return sqrt(sum_deviation/(n-start));

}

int get_thread_num() {
	char *thread_num_str = getenv("THREAD_NUM");
	if (!thread_num_str)
		return 1;
	else
		return atoi(thread_num_str);
}

double runtime_diff_ms(struct timespec *start, struct timespec *end) {
	return ( end->tv_sec - start->tv_sec ) * 1000.0 + (double)( end->tv_nsec - start->tv_nsec ) / 1E6;
}

inline __attribute__((always_inline))
void print_progress_gauge(unsigned read, unsigned total) {
	const static char *progress_gauge_equal = "====================";
	const static char *progress_gauge_empty = "                    ";
	const static int size = 20;
	const static int resolution = 100/20;
	if ((read-1) % (total/100) != 0) return;
	int percent = ((read*100) / total);
	int pres = percent / resolution;
	fprintf(stderr, "  [%.*s%.*s] %3d%%\r", pres, progress_gauge_equal,
		size - pres, progress_gauge_empty, percent);
}

void handle_sigsegv(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stdout, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDOUT_FILENO);
  exit(1);
}

void init_geos() {
	initGEOS(geos_messages, geos_messages);
	signal(SIGSEGV, handle_sigsegv);
	signal(SIGABRT, handle_sigsegv);
}

GEOSContextHandle_t init_geos_context() {
	return initGEOS_r(geos_messages, geos_messages);
}

_Thread_local char _print_size_aux[100];
const char *get_readable_size(size_t  size)
{                   
    static const char *SIZES[] = { "B", "kB", "MB", "GB", "TB" };
    size_t div = 0;
    size_t rem = 0;

    while (size >= 1024 && div < (sizeof SIZES / sizeof *SIZES)) {
        rem = (size % 1024);
        div++;   
        size /= 1024;
    }

	snprintf(_print_size_aux, 100, "%.1f %s",
		(float)size + (float)rem / 1024.0, SIZES[div]);

	return _print_size_aux;
}

