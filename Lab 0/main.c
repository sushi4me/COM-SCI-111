#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>

/*
	* <fcntl.h> controls file options
	* <getopt.h> parses command-line arguments
	* <signal.h> defines symbolic constants
	* <unistd.h> not sure if repeats <fcntl.h>
*/

// Usage message printed upon incorrect use
void usage() {
	printf("lab0 [OPTIONS] -i file1 -o file2\n");
}

void read_and_write(int i, int o) {
	char* buffer = (char*) malloc(sizeof(char));
	ssize_t status = read(i, buffer, 1);
	while(status > 0) {
		write(o, buffer, 1);
		status = read(i, buffer, 1);
	}
	free(buffer);
}

void catch_handler(int sig) {
	if(sig == SIGSEGV) {
		fprintf(stderr, "Handler called for SIGSEGV!\n");
		exit(3);
	}
}

int main(int argc, char** argv) {
	int opt = 0;
	int segfault_flag = 0;
	int input_return_value = 0, output_return_value = 0;

	static struct option long_options[] = {
		{"input",	required_argument,	0,	'i'},
		{"output",	required_argument,	0,	'o'},
		{"segfault",	no_argument,		0,	's'},
		{"catch",	no_argument,		0,	'c'},
		{0,		0,			0,	0}
	};

	/*
		* optarg is from getopt.h
		* O_RDONLY := read only
		* For creat(2): mode_t mode = SIRUSR | S_IWUSR | S_IRGRP | S_IROTH
	*/	
	while((opt = getopt_long(argc, argv, "sci:o:", long_options, NULL)) != -1) {
		printf("Option: %d \n", opt);
		switch(opt) {
			case 'i':
				printf("Intput!\n");
				input_return_value = open(optarg, O_RDONLY);
				if(input_return_value != -1) {
					close(0);
					dup(input_return_value);
					close(input_return_value);
				}
				else {
					fprintf(stderr, "Unable to open the input file: %s\n", optarg);
					perror("Unable to open the input file.");
					exit(1);
				}
				break;
			case 'o':
				printf("Output!\n");
				output_return_value = creat(optarg, 0666);
				if(output_return_value != -1) {
					close(1);
					dup(output_return_value);
					close(output_return_value);
				}
				else {
					fprintf(stderr, "Unable to create the output file: %s\n", optarg);
					perror("Unable to create the output file.");
					exit(2);
				}				
				break;			
			case 's':
				printf("Segfault!\n");
				segfault_flag = 1;
				break;
			case 'c':
				printf("Catch!\n");
				signal(SIGSEGV, catch_handler);
				break;
			default:
				printf("Invalid\n");
		}
	}

	// If segfault option is chosen then we should try to store to a NULL pointer
	if(segfault_flag == 1) {
		char* segfault_ptr = NULL;
		*segfault_ptr = 17;
	}

	// Read and write from input to output
	read_and_write(0, 1);

	exit(0);
}
