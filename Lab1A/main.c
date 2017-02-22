/*-----------------------------------------------------------------------------
	Name:	Nathan Kong
	UID:	204 401 093
	Title:	Lab 1A
-----------------------------------------------------------------------------*/

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <wait.h>

#define TRUE 1
#define FALSE 0

/* global variables */
int FILE_FLAG = 0;
int* fd_array;
int MAX_FILES = 10*sizeof(int);
int MAX_PID = 10*sizeof(int);
int NUM_FILES = 0;
int NUM_PID = 0;
int* pid_array;
int VERBOSE_FLAG = 0;

/* helper functions */
void reallocate_fd_array() {
	if (NUM_FILES == MAX_FILES) {
		MAX_FILES *= 2;
		fd_array = realloc(fd_array, MAX_FILES);
		if (fd_array == NULL) {
			fprintf (stderr, "ERROR: Could not reallocate space.\n");
			exit(EXIT_FAILURE);
		}
	}

	return;
}

void execute_command(char* arg_array[]) {
	int stderr_fd = dup(STDERR_FILENO);
	if (stderr_fd < 0) {
		fprintf (stderr, "ERROR: Error with 'dup' and STDERR.\n");
		exit(EXIT_FAILURE);
	}

	if (dup2(fd_array[atoi(arg_array[0])], STDIN_FILENO) < 0 || 
	    dup2(fd_array[atoi(arg_array[1])], STDOUT_FILENO) < 0 || 
	    dup2(fd_array[atoi(arg_array[2])], STDERR_FILENO) < 0) 
	{
		dup2(stderr_fd, STDERR_FILENO);
		close(stderr_fd);
		fprintf (stderr, "ERROR: dup2 failed to process.\n");
		exit(EXIT_FAILURE);
	}

	/* the fourth [3] index should be the bash command */
	execvp(arg_array[3], &arg_array[3]);
	_exit(errno);
}

bool isOption(int index, char* argv[]) {
	if (argv[index][0] != '-' && argv[index][1] != '-')
		return FALSE;
	else
		return TRUE;
}

/* option functions */
void file_function() {
	/* check verbose */	
	if (VERBOSE_FLAG)
		printf ("--%sonly %s\n", 
		(FILE_FLAG == O_RDONLY) ? "rd" : "wr", optarg);
	/* check if file exists */
	if (access(optarg, F_OK) < 0) {
		fprintf (stderr, "ERROR: %s does not exist.\n", optarg);
		exit(EXIT_FAILURE);
	}
	
	/* attempt to open the file */
	fd_array[NUM_FILES] = open (optarg, FILE_FLAG, S_IRUSR | S_IWUSR | S_IROTH);
	if (fd_array[NUM_FILES] < 0) {
		fprintf (stderr, "ERROR: Could not open %s.\n", optarg);
		exit(EXIT_FAILURE);
	}

	FILE_FLAG = 0;
	NUM_FILES++;
	
	/* check if fd_array is full */
	reallocate_fd_array();

	return;
	
}

void command_function(int argc, char* argv[]) {
	/* optind points to the NEXT arg to be processed */
	int arg_index = optind - 1;
	int MAX_ARG = 10;
	int NUM_ARG = 0;

	/* allocate space for the command arguments to keep track */
	char** arg_array = (char**) malloc (MAX_ARG*sizeof(char*));
	if (arg_array == NULL) {
		fprintf (stderr, "ERROR: Could not allocate space.\n");
		exit(EXIT_FAILURE);
	}

	/* we want to add arguments to the arg_array until we reach the end or next command */
	while (1) {
		if ((arg_index == argc) || isOption(arg_index, argv)) {
			optind = arg_index;	// point optind to the next command
			arg_array[NUM_ARG] = NULL;
			break;
		}
	
		/* place whatever optind points to into arg_array */
		arg_array[NUM_ARG++] = argv[arg_index++];

		/* reallocate arg_array if full, not the same as fd_array since these are not global */
		if (NUM_ARG == MAX_ARG) {
			MAX_ARG *= 2;
			arg_array = (char**) realloc (arg_array, MAX_ARG);
			if (arg_array == NULL) {
				fprintf (stderr, "ERROR: Could not reallocate space.\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	/* we know commands should be: --command # # # <command> ARG1 ARG2 ... */
	if (NUM_ARG < 4) {
		fprintf (stderr, "ERROR: Minimum arguments required is 4.\n");
		return;
	}
	
	/* check that the 3 file descriptors are present */
	for (int i = 0; i < 3; i++) {
		int fd = atoi(arg_array[i]);
		if (fd < 0 || fd >= NUM_FILES) {
			fprintf (stderr, "ERROR: The file descriptor does not exist.\n");
			return;

		}
	}

	/* at this point we have the command */
	if (VERBOSE_FLAG) {
		printf ("--command");
		//int i = 0;
		for (int i = 0; arg_array[i] != NULL; ++i)	
			printf(" %s", arg_array[i]);
		printf ("\n");
	}

	/* fork process and execute */
	int pid = fork();
	if (pid == 0) {
		execute_command(arg_array);		
	}
	else if (pid < 0) {
		fprintf (stderr, "ERROR: Failed for fork child process.\n");
		exit(errno);	
	}
	else {
		/* list of children processes */
		pid_array[NUM_PID++] = pid;
	}
	
	free(arg_array);
}

/* main function */
int main(int argc, char* argv[]) {
	/* allocate space for file descriptors to keep track */
	fd_array = malloc(MAX_FILES);
	pid_array = malloc(MAX_PID);

	if (fd_array == NULL) {
		fprintf (stderr, "ERROR: Could not allocate space.\n");
		exit(EXIT_FAILURE);
	}

	if (pid_array == NULL) {
		fprintf (stderr, "ERROR: Could not allocate space.\n");
		exit(EXIT_FAILURE);
	}

	static struct option long_opts[] =
	{ 
		{"command",	required_argument,	0,	'c'},
		{"rdonly",	required_argument,	0,	O_RDONLY},
		{"wronly",	required_argument,	0,	O_WRONLY},
		{"verbose",	no_argument,		0,	'v'},
		{0,		0,			0,	0}
	};

	/* option parsing */
	int opt;
	while ((opt = getopt_long(argc, argv, "", long_opts, NULL)) != -1) {
		switch (opt) {
			case 'c':
				command_function(argc, argv);				
				break;
			case 'v':
				VERBOSE_FLAG = 1;
				break;
			case O_RDONLY:
			case O_WRONLY:
				FILE_FLAG = opt;
				file_function();
				break;
		}
	}

	/* clean up */
	for (int i = 0; i < NUM_FILES; i++)
		close(fd_array[i]);

	free(fd_array);
	free(pid_array);

	exit(0);
}
