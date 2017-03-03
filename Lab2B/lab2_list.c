#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#define  NS_PER_S	1000000000

#include <errno.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "SortedList.h"

// Globals
SortedList_t *list;
SortedListElement_t *elements;

static long long mutex_lock_time = 0;
const char char_set[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
static int n_threads = 1;
static int n_iterations = 1;
static int n_lists = 1;
char opt_sync = 0;

static pthread_mutex_t *m;
static int *spin_lock;

// Functions
int hashingFunction(const char* key) {
	int num = 0;
	for(int i = 0; i < sizeof(key); i++)
		num += key[i];
	return num%n_lists;
}

void runTestThread(void *arg)
{
	int* p_thread_num =(int *)arg;
	int thread_num = *p_thread_num;

	long start = thread_num*n_iterations;
	long end = start + n_iterations;

	// Insert elements into multilist
	for(long i = start; i < end; i++) {
		int l_num = hashingFunction((elements + i)->key);

		// Lock list
		if(opt_sync == 'm') {
			struct timespec m_start, m_end;
			if(clock_gettime(CLOCK_MONOTONIC, &m_start) == -1) {
        			fprintf(stderr, "ERROR: Could not get start_time.\n");
        			exit(1);
    			}
			pthread_mutex_lock(&m[l_num]);
			if(clock_gettime(CLOCK_MONOTONIC, &m_end) == -1) {
        			fprintf(stderr, "ERROR: Could not get end_time.\n");
        			exit(1);
    			}
			long long start = m_start.tv_sec*NS_PER_S + m_start.tv_nsec;
			long long end = m_end.tv_sec*NS_PER_S + m_end.tv_nsec;
			mutex_lock_time += end - start;
		}
		else if(opt_sync == 's')
			while(__sync_lock_test_and_set(&spin_lock[l_num], 1));

		// INSERT ELEMENT
		SortedList_insert(&list[l_num], elements + i);
		
		// Release list
		if(opt_sync == 'm')
			pthread_mutex_unlock(&m[l_num]);
		else if(opt_sync == 's')
			__sync_lock_release(&spin_lock[l_num]);
	}

	// FIND THE LENGTH OF THE LIST
	for(int i = 0; i < n_lists; i++) {	
		// Lock list
		if(opt_sync == 'm') {
			struct timespec m_start, m_end;
			if(clock_gettime(CLOCK_MONOTONIC, &m_start) == -1) {
        			fprintf(stderr, "ERROR: Could not get start_time.\n");
        			exit(1);
    			}
			pthread_mutex_lock(&m[i]);
			if(clock_gettime(CLOCK_MONOTONIC, &m_end) == -1) {
        			fprintf(stderr, "ERROR: Could not get end_time.\n");
        			exit(1);
    			}
			long long start = m_start.tv_sec*NS_PER_S + m_start.tv_nsec;
			long long end = m_end.tv_sec*NS_PER_S + m_end.tv_nsec;
			mutex_lock_time += end - start;
		}
		else if(opt_sync == 's')
			while(__sync_lock_test_and_set(&spin_lock[i], 1));

		SortedList_length(&list[i]);
		
		// Release list
		if(opt_sync == 'm')
			pthread_mutex_unlock(&m[i]);
		else if(opt_sync == 's')
			__sync_lock_release(&spin_lock[i]);
	}

	char *key_check = malloc(sizeof(char) * 5);
	SortedListElement_t *a;

	// Remove inserted keys
	for(long i = start; i < end; i++) {
		strcpy(key_check,(elements + i)->key);
		int l_num = hashingFunction(key_check);

		// Lock list
		if(opt_sync == 'm') {
			struct timespec m_start, m_end;
			if(clock_gettime(CLOCK_MONOTONIC, &m_start) == -1) {
        			fprintf(stderr, "ERROR: Could not get start_time.\n");
        			exit(1);
    			}
			pthread_mutex_lock(&m[l_num]);
			if(clock_gettime(CLOCK_MONOTONIC, &m_end) == -1) {
        			fprintf(stderr, "ERROR: Could not get end_time.\n");
        			exit(1);
    			}
			long long start = m_start.tv_sec*NS_PER_S + m_start.tv_nsec;
			long long end = m_end.tv_sec*NS_PER_S + m_end.tv_nsec;
			mutex_lock_time += end - start;
		}
		else if(opt_sync == 's')
			while(__sync_lock_test_and_set(&spin_lock[l_num], 1));

		// LOOK UP ELEMENT IN LIST
		a = SortedList_lookup(&list[l_num], key_check);
		
		// Release list
		if(opt_sync == 'm')
			pthread_mutex_unlock(&m[l_num]);
		else if(opt_sync == 's')
			__sync_lock_release(&spin_lock[l_num]);

		// Lookup failed
		if(!a) {
			fprintf(stderr, "ERROR: Element could not be found.\n");
			exit(1);
		}

		// Lock list
		if(opt_sync == 'm') {
			struct timespec m_start, m_end;
			if(clock_gettime(CLOCK_MONOTONIC, &m_start) == -1) {
        			fprintf(stderr, "ERROR: Could not get start_time.\n");
        			exit(1);
    			}
			pthread_mutex_lock(&m[l_num]);
			if(clock_gettime(CLOCK_MONOTONIC, &m_end) == -1) {
        			fprintf(stderr, "ERROR: Could not get end_time.\n");
        			exit(1);
    			}
			long long start = m_start.tv_sec*NS_PER_S + m_start.tv_nsec;
			long long end = m_end.tv_sec*NS_PER_S + m_end.tv_nsec;
			mutex_lock_time += end - start;
		}
		else if(opt_sync == 's')
			while(__sync_lock_test_and_set(&spin_lock[l_num], 1));

		// Work around for error which says l_num is an improper subscript
		pthread_mutex_t *m_temp = (m+l_num);

		// DELETE ELEMENT FROM LIST
		int m = SortedList_delete(a);

		// Release list
		if(opt_sync == 'm')
			pthread_mutex_unlock(m_temp);
		else if(opt_sync == 's')
			__sync_lock_release(&spin_lock[l_num]);

		// Delete failed
		if(m != 0) {
			fprintf(stderr, "ERROR: Element could not be deleted.\n");
			exit(1);
		}
	}
}

// Main
int main(int argc, char** argv)
{
	int opt;

	static struct option long_options[] = {
		{"threads",     required_argument, 	0, 	't'},
		{"iterations",	required_argument, 	0,	'i'},
		{"yield",       required_argument, 	0, 	'y'},
		{"sync",        required_argument, 	0, 	's'},
		{"list",	required_argument,	0,	'l'},
		{0, 		0, 			0, 	0}
	};

	while((opt = getopt_long(argc, argv, "", long_options, NULL)) != -1) {
		switch(opt) {
			case 't':
				n_threads = atoi(optarg);
				break;
			case 'i':
				n_iterations = atoi(optarg);
				break;
			case 'y':
				for(int i = 0; i < strlen(optarg); i++) {
					if(optarg[i] == 'i')
						opt_yield |= INSERT_YIELD;
					else if(optarg[i] == 'd')
						opt_yield |= DELETE_YIELD;
					else if(optarg[i] == 'l')
						opt_yield |= LOOKUP_YIELD;
				}
				break;
			case 's':
				opt_sync = optarg[0];
				break;
			case 'l':
				n_lists = atoi(optarg);
				break;			
			default:
				fprintf(stderr, "ERROR: Invalid argument.\n");
				break;
		}
	}
	long n_elements = n_threads*n_iterations;

	// Allocate locks for each list given sync
	if(opt_sync == 'm') {
		m = malloc(sizeof(pthread_mutex_t)*n_lists);
		if(!m) {
			fprintf(stderr, "ERROR: Malloc failed for mutex.\n");
			exit(1);
		}
		for(int i = 0; i < n_lists; i++) {
			pthread_mutex_init(&m[i], NULL);
		}	
	}
	else if(opt_sync == 's') {
		spin_lock = malloc(sizeof(int)*n_lists);
		if(!spin_lock) {
			fprintf(stderr, "ERROR: Malloc failed for spin_lock.\n");
			exit(1);
		}
	}

	// Allocate lists
	list = malloc(sizeof(SortedList_t)*n_lists);
	if(!list) {
		fprintf(stderr, "ERROR: Malloc failed for list.\n");
		exit(1);
	}

	// Allocate elements to be inserted into lists
	elements = malloc(sizeof(SortedListElement_t)*n_elements);
	if(!elements) {
		fprintf(stderr, "ERROR: Malloc failed for elements.\n");
		exit(1);
	}


	for(long n = 0; n < n_elements; n++) {
    		// Generate key
		char *rand_key = malloc(sizeof(char)*5);
        	if(rand_key == NULL) {
			fprintf(stderr, "ERROR: Malloc failed for key generator.\n");
			exit(1);
		}
		rand_key[0] = char_set[rand()%(int)(sizeof(char_set) - 1)];
		rand_key[1] = char_set[rand()%(int)(sizeof(char_set) - 1)];
		rand_key[2] = char_set[rand()%(int)(sizeof(char_set) - 1)];
		rand_key[3] = char_set[rand()%(int)(sizeof(char_set) - 1)];
		rand_key[4] = '\0';

		(elements + n)->key = rand_key;
	}

    	int *thread_num = malloc(sizeof(int)*n_threads);
    	if(!thread_num) {
    		fprintf(stderr, "ERROR: Malloc failed for thread num.\n");
    		exit(1);
    	}

	// Thread arguments
    	for(int i = 0; i < n_threads; i++)
    		thread_num[i] = i;

    	// Start timing
	struct timespec time_start, time_end;
    	if(clock_gettime(CLOCK_MONOTONIC, &time_start) == -1) {
        	fprintf(stderr, "ERROR: Could not get start_time.\n");
        	exit(1);
    	}

    	pthread_t *pid = malloc(sizeof(pthread_t)*n_threads);
    	if(!pid) {
        	fprintf(stderr, "ERROR: Malloc failed for pid.\n");
        	exit(1);
    	}

    	// Create threads
    	for(int i = 0; i < n_threads; i++) {
        	int thread_status = pthread_create(&pid[i], 
			NULL, 
			(void *)runTestThread, 
			(void *)(thread_num + i));

        	if(thread_status) {
            		fprintf(stderr, "ERROR: Could not create pthread.\n");
            		exit(1);
        	}
    	}
    	
	// Join threads
	for(int i = 0; i < n_threads; i++) {
        	int thread_status = pthread_join(pid[i], NULL);
        	if(thread_status) {
            		fprintf(stderr, "ERROR: Could not join thread.\n");
            		exit(1);
        	}
    	}

    	// Stop timing
    	if(clock_gettime(CLOCK_MONOTONIC, &time_end) == -1) {
        	fprintf(stderr, "ERROR: Could not get end_time.\n");
        	exit(1);
    	}

    	// Get length
	for(int i = 0; i < n_lists; i++) {
    		long long list_length = SortedList_length(&list[i]);

    		if(list_length != 0) {
        		fprintf(stderr, "ERROR: Length of list is not ZERO.\n");
        		exit(1);
    		}
	}

	// Print CVS
    	fprintf(stdout, "list");
	switch(opt_yield) {
        	case 0:
            		fprintf(stdout, "-none");
            		break;
        	case 1:
            		fprintf(stdout, "-i");
            		break;
        	case 2:
            		fprintf(stdout, "-d");
            		break;
        	case 3:
            		fprintf(stdout, "-id");
            		break;
        	case 4:
            		fprintf(stdout, "-l");
            		break;
        	case 5:
            		fprintf(stdout, "-il");
            		break;
        	case 6:
            		fprintf(stdout, "-dl");
            		break;
        	case 7:
            		fprintf(stdout, "-idl");
            		break;
        	default:
            		break;
    	}
	switch(opt_sync) {
        	case 0:
            		fprintf(stdout, "-none");
            		break;
        	case 's':
            		fprintf(stdout, "-s");
            		break;
        	case 'm':
            		fprintf(stdout, "-m");
           		break;
        	default:
            		break;
    	}

	// Calculate times
	long long n_operations = n_threads*n_iterations*3;
	long long start_time = (long long)(time_start.tv_sec*NS_PER_S + time_start.tv_nsec);
	long long end_time = (long long)(time_end.tv_sec*NS_PER_S + time_end.tv_nsec);
	long long elapsed_time = end_time - start_time;
	long long average_time_per_operation = elapsed_time/n_operations;
	long long average_lock_wait_time = mutex_lock_time/n_operations;

	if(opt_sync == 'm')
		fprintf(stdout, ",%d,%d,%d,%lld,%lld,%lld,%lld\n", 
			n_threads, 
			n_iterations, 
			n_lists,
			n_operations, 
			elapsed_time, 
			average_time_per_operation,
			average_lock_wait_time);
	else
		fprintf(stdout, ",%d,%d,%d,%lld,%lld,%lld\n", 
			n_threads, 
			n_iterations, 
			n_lists,
			n_operations, 
			elapsed_time, 
			average_time_per_operation);

    	free(pid);
    	free(list);
    	free(elements);
    	free(thread_num);

    	exit(0);
}
