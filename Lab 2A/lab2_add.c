#define _GNU_SOURCE

#include <getopt.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Globals
static long long n_threads = 1;
static long long n_iterations = 1;

// Functions
void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	*pointer = sum;
}

void runThreadTest(void) {
	// Start time
	struct timespec t_start, t_end;
	clock_gettime(CLOCK_MONOTONIC, &t_start);

	// Ending time
	clock_gettime(CLOCK_MONOTONIC, &t_end);

	// Calculate times
}

// Main
int main(int argc, char** argv) {
	int opt = 0;
	int opt_index = 0;

	static struct option long_options[] = {
		{"threads",	required_argument,	0,	't'},
		{"iterations",	required_argument,	0,	'i'},
		{"yield",	no_argument,		0,	'y'},
		{0,		0,			0,	0}
	};
	
	while((opt = getopt_long(argc, argv, "", long_options, &opt_index) != -1)) {
		switch(opt) {
			case 't':
				n_threads = atoi(optarg);
				break;
			case 'i':
				n_iterations = atoi(optarg);
				break;
			case 'y':
				break;
			default:
				exit(2);
				break;
		}
	}

	runThreadTest();
	return 0;
}
