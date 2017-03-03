#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define  NS_PER_S	1000000000

#include <getopt.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Globals
static char* test = "add-none";
static long long n_threads = 1;
static long long n_iterations = 1;
static long long result = 0;
static int spin_lock = 0;

bool opt_yield = false;
bool opt_sync = false;

pthread_mutex_t m;

// Functions
void (*opt_lock)(long long *, long long);

void add(long long *pointer, long long value) {
	long long sum = *pointer + value;
	if (opt_yield)
		if(sched_yield() != 0) {
			fprintf(stderr, "ERROR: sched_yield failed.\n");
			exit(2);
		}
	*pointer = sum;
}

void add_m(long long *pointer, long long value) {
	pthread_mutex_lock(&m);
	add(&result, value);
	pthread_mutex_unlock(&m);
	/* Lock the variable and unlock upon completion. */
}

void add_s(long long *pointer, long long value) {
	while(__sync_lock_test_and_set(&spin_lock, value));
	add(pointer, value);
	__sync_lock_release(&spin_lock);
	/* Spin until the lock is released. */
}

void add_c(long long *pointer, long long value) {
	long long temp, sum;
	do {
		temp = *pointer;
		if (opt_yield)
			if(sched_yield() != 0) {
				fprintf(stderr, "ERROR: sched_yield failed.\n");
				exit(2);
			}
		sum = temp + value;
	}
	while(__sync_val_compare_and_swap(pointer, temp, sum) != temp);
	/* If current value of pointer is temp, then write sum into pointer. */
}

void wrapperAddFunction(void *arg) {
	for(int i = 0; i < n_iterations; i++)
		(*opt_lock)(&result, 1);

	for(int i = 0; i < n_iterations; i++)
		(*opt_lock)(&result, -1);
}

void runThreadTest(void) {
	// Start time
	struct timespec t_start, t_end;
	clock_gettime(CLOCK_MONOTONIC, &t_start);

	// Run test
	pthread_t* p = (pthread_t*) malloc(n_threads*sizeof(pthread_t));
	
	for(int i = 0; i < n_threads; i++)
		if(pthread_create(&p[i], NULL, (void*)wrapperAddFunction, NULL)) {
			fprintf(stderr, "ERROR: Creating threads.\n");
			exit(1);
		}

	for(int i = 0; i < n_threads; i++)
		if(pthread_join(p[i], NULL)) {
			fprintf(stderr, "ERROR: Joining threads.\n");
			exit(1);
		}

	// Ending time
	clock_gettime(CLOCK_MONOTONIC, &t_end);

	// Calculate times
	long long n_operations = n_threads*n_iterations*2;
	long long start_time = (long long)(t_start.tv_sec*NS_PER_S + t_start.tv_nsec);
	long long end_time = (long long)(t_end.tv_sec*NS_PER_S + t_end.tv_nsec);
	long long elapsed_time = end_time - start_time;
	long long thread_time = elapsed_time/n_iterations;

	printf("%s,%lld,%lld,%lld,%lld,%lld,%lld\n", 
		test, 
		n_threads, 
		n_iterations,
		n_operations, 
		elapsed_time,
		thread_time,
		result);
	
	free(p);
}

// Main
int main(int argc, char** argv) {
	int opt;
	char sync_opt = 0;
	opt_lock = &add;

	static struct option long_opts[] = {
		{"threads",	required_argument,	0,	't'},
		{"iterations",	required_argument,	0,	'i'},
		{"yield",	no_argument,		0,	'y'},
		{"sync",	required_argument,	0,	's'},
		{0,		0,			0,	0}
	};
	
	while((opt = getopt_long(argc, argv, "", long_opts, NULL)) != -1) {
		switch(opt) {
			case 't':
				n_threads = atoi(optarg);
				break;
			case 'i':
				n_iterations = atoi(optarg);
				break;
			case 'y':
				opt_yield = true;
				test = "add-yield-none";
				break;
			case 's':
				opt_sync = true;
				sync_opt = *optarg;
				break;
			default:
				exit(2);
				break;
		}
	}

	if(opt_sync) {
		switch(sync_opt) {
			case 'm':
				pthread_mutex_init(&m, NULL);
				opt_lock = &add_m;
				if(opt_yield)
					test = "add-yield-m";
				else
					test = "add-m";
				break;
			case 's':
				opt_lock = &add_s;
				if(opt_yield)
					test = "add-yield-s";
				else
					test = "add-s";
				break;
			case 'c':
				opt_lock = &add_c;
				if(opt_yield)
					test = "add-yield-c";
				else
					test = "add-c";
				break;
			default:
				break;
		}
	}
	runThreadTest();
	return 0;
}
