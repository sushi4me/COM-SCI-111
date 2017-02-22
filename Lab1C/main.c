/******************************************************************************
*	Name:	Nathan Kong
*	UID:	204 401 093
*	Title:	Lab 1C
******************************************************************************/

#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#define TRUE 1
#define FALSE 0

struct process {
	pid_t p_pid;
	char* p_args[100];
};

/* GLOBAL VARIABLES */
int FILE_FLAG = 0;
int* fd_array;
struct rusage m_usage;
int MAX_FILES = 10*sizeof(int);
int MAX_PID = 10*sizeof(int);
int MAX_PROCESS = 10;
const double MILLION = 1000000;
const mode_t mode = 0644;
int NUM_FILES = 0;
int NUM_PID = 0;
int NUM_PROCESS = 0;
int* pid_array;
int* pipe_array;
double prev_user_time;
double prev_sys_time;
int PROFILE_FLAG = 0;
struct process* process_array;
static double* total_user_time;
static double* total_sys_time;
int VERBOSE_FLAG = 0;

/* PROTOTYPES */
bool pipe_check(int fd);
int get_pipe_end(int fd);
void f_profile(int m_who);

/* HELPER FUNCTIONS */
void catch_handler(int sig) {
	fprintf(stderr, "ERROR: Signal %d caught.\n", sig);
	exit(sig);
}

void execute_command(char* arg_array[]) {
	int stderr_fd = dup(STDERR_FILENO);
	if (stderr_fd < 0) {
		fprintf(stderr, "ERROR: Error with 'dup' and STDERR.\n");
		exit(EXIT_FAILURE);
	}

	/* close the other end of the pipe */
	if (pipe_array[atoi(arg_array[0])])
        	close((fd_array[atoi(arg_array[0])+1]));
        if (pipe_array[atoi(arg_array[1])])
                close((fd_array[atoi(arg_array[1])-1]));
	if (pipe_array[atoi(arg_array[2])])
		close((fd_array[atoi(arg_array[2])-1]));

	if (dup2(fd_array[atoi(arg_array[0])], STDIN_FILENO) < 0 || 
	    dup2(fd_array[atoi(arg_array[1])], STDOUT_FILENO) < 0 || 
	    dup2(fd_array[atoi(arg_array[2])], STDERR_FILENO) < 0) 
	{
		dup2(stderr_fd, STDERR_FILENO);
		close(stderr_fd);
		fprintf(stderr, "ERROR: dup2 failed to process.\n");
		exit(EXIT_FAILURE);
	}

	execvp(arg_array[3], &arg_array[3]);
	_exit(errno);
}

int get_pipe_end(int fd) {
	/* if fd is 1 then the other end is infront otherwise it is before */
	return pipe_array[fd] == 1 ? fd + 1 : fd - 1;
}

void ignore_handler(int sig) {
	return;
}

bool isOption(int index, char* argv[]) {
	if (argv[index][0] == '-' && argv[index][1] == '-')
		return TRUE;
	else
		return FALSE;
}

bool pipe_check(int fd) {
	/* fd is out of bounds */
	if (pipe_array[fd] < 0 || pipe_array[fd] > 1)
		return FALSE;
	/* fd is one of the two ends, if the current is not one then check before */
	if (pipe_array[fd] == 1 || (fd > 0 && pipe_array[fd - 1] == 1))
		return TRUE;
	return FALSE;
}

void reallocate_arrays() {
	if (NUM_FILES == MAX_FILES) {
		MAX_FILES *= 2;
		fd_array = realloc(fd_array, MAX_FILES);
		pipe_array = realloc(pipe_array, MAX_FILES);
		if (fd_array == NULL || pipe_array == NULL) {
			fprintf(stderr, "ERROR: Could not reallocate space.\n");
			exit(EXIT_FAILURE);
		}
	}

	return;
}

/* OPTION FUNCTIONS */
void f_command(int argc, char* argv[]) {
	/* optind points to the NEXT arg to be processed */
	int arg_index = optind - 1;
	int MAX_ARG = 10;
	int NUM_ARG = 0;

	/* allocate space for the command arguments to keep track */
	char** arg_array = (char**) malloc (MAX_ARG*sizeof(char*));
	if (arg_array == NULL) {
		fprintf(stderr, "ERROR: Could not allocate space.\n");
		exit(EXIT_FAILURE);
	}

	/* we want to add arguments to the arg_array until we reach the end or next command */
	while (1) {
		if ((arg_index == argc) || isOption(arg_index, argv)) {
			optind = arg_index;	// point optind to the next command
			process_array[NUM_PROCESS].p_args[NUM_ARG] = NULL;
			arg_array[NUM_ARG] = NULL;
			break;
		}
	
		/* place whatever optind points to into arg_array */
		process_array[NUM_PROCESS].p_args[NUM_ARG] = argv[arg_index];		
		arg_array[NUM_ARG++] = argv[arg_index++];

		/* reallocate arg_array if full, not the same as fd_array since these are not global */
		if (NUM_ARG == MAX_ARG) {
			MAX_ARG *= 2;
			arg_array = (char**) realloc (arg_array, MAX_ARG);
			if (arg_array == NULL) {
				fprintf(stderr, "ERROR: Could not reallocate space.\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	/* we know commands should be: --command # # # <command> ARG1 ARG2 ... */
	if (NUM_ARG < 4) {
		fprintf(stderr, "ERROR: Minimum arguments required is 4.\n");
		return;
	}
	
	/* check that the 3 file descriptors are present */
	for (int i = 0; i < 3; i++) {
		int fd = atoi(arg_array[i]);
		if (fd < 0 || fd >= NUM_FILES) {
			fprintf(stderr, "ERROR: The file descriptor does not exist.\n");
			return;
		}
	}

	/* at this point we have the command */
	if (VERBOSE_FLAG) {
		printf("--command");
		//int i = 0;
		for (int i = 0; arg_array[i] != NULL; ++i)	
			printf(" %s", arg_array[i]);
		printf("\n");
	}

	/* fork process and execute */
	int pid = fork();
	if (pid == 0) {
		execute_command(arg_array);		
	}
	else if (pid < 0) {
		fprintf(stderr, "ERROR: Failed for fork child process.\n");
		exit(errno);	
	}
	else {
		/* list of children processes */
		pid_array[NUM_PID++] = pid;
		process_array[NUM_PROCESS++].p_pid = pid;

		/* close pipe end */
		if (pipe_array[atoi(arg_array[0])]){
                	close(fd_array[atoi(arg_array[0])]);
                        fd_array[atoi(arg_array[0])] = -1;
                }
		if (pipe_array[atoi(arg_array[1])]){
                	close(fd_array[atoi(arg_array[1])]);
                        fd_array[atoi(arg_array[1])] = -1;
                }
		if (pipe_array[atoi(arg_array[2])]){
                	close(fd_array[atoi(arg_array[2])]);
                        fd_array[atoi(arg_array[2])] = -1;
                }
	}
	
	free(arg_array);
	if (PROFILE_FLAG)
		f_profile(RUSAGE_SELF);
}

void f_catch() {
	int signal_no = atoi(optarg);
	if (VERBOSE_FLAG)
		printf("--catch %d\n", signal_no);	
	/* if the signal is caught, print to stderr and exit */
	signal(signal_no, catch_handler);

	if (PROFILE_FLAG)
		f_profile(RUSAGE_SELF);
}

void f_close() {
	int fd = atoi(optarg);
	if (VERBOSE_FLAG)
		printf("--close %d\n", fd);
	if (fd > NUM_FILES) {
		fprintf(stderr, "ERROR: Invalid file descriptor.\n");
		exit(EXIT_FAILURE);
	}
	fd_array[fd] = -1;
	close(fd_array[fd]);

	if (PROFILE_FLAG)
		f_profile(RUSAGE_SELF);
}

void f_crash() {
	if (VERBOSE_FLAG)
		printf("--abort\n");
	int* ptr = NULL;
	*ptr = 2017;
}

void f_default() {
	int signal_no = atoi(optarg);
	if (VERBOSE_FLAG)
		printf("--default %d\n", signal_no);
	/* if the signal is caught, use the DEFAULT handler */
	signal(signal_no, SIG_DFL);

	if (PROFILE_FLAG)
		f_profile(RUSAGE_SELF);
}

void f_ignore() {
	int signal_no = atoi(optarg);
	if (VERBOSE_FLAG)
		printf("--ignore %d\n", signal_no);
	/* if the signal is caught, ignore it -- we do NOT use SIG_IGN, it does not ignore SIGKILL */
	//signal(signal_no, ignore_handler);
	signal(signal_no, SIG_IGN);

	if (PROFILE_FLAG)
		f_profile(RUSAGE_SELF);
}

void f_openfile() {
	/* check verbose */	
	if (VERBOSE_FLAG) {
		if ((FILE_FLAG | O_RDWR) == FILE_FLAG)
			printf("--rdwr %s\n", optarg);
		else if ((FILE_FLAG | O_WRONLY) == FILE_FLAG)
			printf("--wronly %s\n", optarg);
		else if ((FILE_FLAG | O_RDONLY) == FILE_FLAG)
			printf("--rdonly %s\n", optarg);
	}
	
	/* check if file exists */
	if ((FILE_FLAG | O_CREAT) != FILE_FLAG && access(optarg, F_OK) < 0) {
		fprintf(stderr, "ERROR: %s does not exist.\n", optarg);
		exit(EXIT_FAILURE);
	}
	
	/* attempt to open the file */
	fd_array[NUM_FILES] = open(optarg, FILE_FLAG, mode);//S_IRUSR | S_IWUSR | S_IROTH);

	if (fd_array[NUM_FILES++] < 0) { //
		fprintf(stderr, "ERROR: Could not open %s.\n", optarg);
		exit(EXIT_FAILURE);
	}

	pipe_array[NUM_FILES] = 0;
	FILE_FLAG = 0;
	
	/* check if fd_array is full */
	reallocate_arrays();
	if (PROFILE_FLAG)
		f_profile(RUSAGE_SELF);

	return;
}

void f_openpipe() {
	int pipe_ends[2];

	if (VERBOSE_FLAG)
		printf("--pipe\n");

	if (pipe(pipe_ends) < 0) {
		fprintf(stderr, "ERROR: Cannot open up a pipe.\n");
		exit(EXIT_FAILURE);
	}

	fd_array[NUM_FILES++] = pipe_ends[0];
	pipe_array[NUM_FILES] = 1;
	fd_array[NUM_FILES++] = pipe_ends[1];
	pipe_array[NUM_FILES] = 1;

	if (PROFILE_FLAG)
		f_profile(RUSAGE_SELF);
}

void f_profile(int m_who) {
	if (getrusage(m_who, &m_usage) < 0) {
		fprintf(stderr, "ERROR: Could not get rusage.\n");
		exit(errno);
	}
	else {
		double user_time = ((double)m_usage.ru_utime.tv_sec 
				+ (double)m_usage.ru_utime.tv_usec/MILLION)
				- prev_user_time;
		double sys_time = ((double)m_usage.ru_stime.tv_sec 
				+ (double)m_usage.ru_stime.tv_usec/MILLION)
				- prev_sys_time;
		*total_user_time += user_time;
		*total_sys_time += sys_time;
		printf("\tUser time: \t\t\t%f s\n", user_time);
		printf("\tSystem time: \t\t\t%f s\n", sys_time);
	}
}

void f_wait() {
	if (VERBOSE_FLAG)
		printf("--wait\n");

	int pid_status, exit_status;
	for (int i = 0; i < NUM_PROCESS; i++) {
		exit_status = waitpid(-1, &pid_status, 0);
		printf("EXIT_STATUS: %d, ", WEXITSTATUS(pid_status));

		int j;
		for (j = 0; j < NUM_PROCESS; j++)
			if (exit_status == process_array[j].p_pid)
				break;

		int k = 0;
		for (; process_array[j].p_args[k] != NULL; k++)
			printf("%s ", process_array[j].p_args[k]);
		printf("\n");
	}

	if (PROFILE_FLAG) {
		printf("Parent Information:\n");		
		f_profile(RUSAGE_SELF);
		printf("Children Information:\n");
		f_profile(RUSAGE_CHILDREN);
	}
}

/* MAIN */
int main(int argc, char* argv[]) {
	/* allocate space for file descriptors to keep track */
	fd_array = malloc(MAX_FILES);
	pid_array = malloc(MAX_PID);
	pipe_array = malloc(MAX_FILES);
	process_array = (struct process*) malloc(MAX_PROCESS*sizeof(struct process));

	if (fd_array == NULL) {
		fprintf(stderr, "ERROR: Could not allocate space for file descriptors.\n");
		exit(EXIT_FAILURE);
	}

	if (pid_array == NULL) {
		fprintf(stderr, "ERROR: Could not allocate space for pids.\n");
		exit(EXIT_FAILURE);
	}

	if (pipe_array == NULL) {
		fprintf(stderr, "ERROR: Could not allocate space for pipes.\n");
		exit(EXIT_FAILURE);
	}

	if (process_array == NULL) {
		fprintf(stderr, "ERROR: Count not allocate space for processes.\n");
		exit(EXIT_FAILURE);
	}

	static struct option long_opts[] =
	{ 
		/* LAB 1A */
		{"command",	required_argument,	0,	'c'},
		{"rdonly",	required_argument,	0,	O_RDONLY},
		{"wronly",	required_argument,	0,	O_WRONLY},
		{"verbose",	no_argument,		0,	'v'},
		/* LAB 1B */
		{"abort",	no_argument,		0,	'a'},
		{"append",	no_argument,		0,	O_APPEND},
		{"catch",	required_argument,	0,	't'},		
		{"cloexec",	no_argument,		0,	O_CLOEXEC},
		{"close",	required_argument,	0,	'l'},		
		{"creat",	no_argument,		0,	O_CREAT},
		{"default",	required_argument,	0,	'd'},		
		{"directory",	no_argument,		0,	O_DIRECTORY},
		{"dsync",	no_argument,		0,	O_DSYNC},		
		{"excl",	no_argument,		0,	O_EXCL},
		{"ignore",	required_argument,	0,	'i'},		
		{"nofollow",	no_argument,		0,	O_NOFOLLOW},
		{"nonblock",	no_argument,		0,	O_NONBLOCK},
		{"pause",	no_argument,		0,	'u'},
		{"pipe",	no_argument,		0,	'p'},
		{"rdwr",	required_argument,	0,	O_RDWR},
		{"rsync",	no_argument,		0,	O_RSYNC},
		{"sync",	no_argument,		0,	O_SYNC},
		{"trunc",	no_argument,		0,	O_TRUNC},
		{"wait",	no_argument,		0,	'w'},
		/* LAB 1C */
		{"profile",	no_argument,		0,	'r'},
		{0,		0,			0,	0}
	};

	/************************************************************************
	* NOTE:
	*	"zero or more file creation flags and file status flags can be
	*	bitwise-or'd in flags"
	************************************************************************/

	/* option parsing */
	int opt;
	while ((opt = getopt_long(argc, argv, "", long_opts, NULL)) != -1) {
		if (PROFILE_FLAG) {
			/* setting up global variables that work even for forks */	
			total_user_time = mmap(NULL, 
				sizeof *total_user_time, 
				PROT_READ | PROT_WRITE, 
				MAP_SHARED | MAP_ANONYMOUS,
				-1, 0);
			*total_user_time = 0;

			total_sys_time = mmap(NULL, 
				sizeof *total_user_time, 
				PROT_READ | PROT_WRITE, 
				MAP_SHARED | MAP_ANONYMOUS,
				-1, 0);
			*total_sys_time = 0;
		
			if (getrusage(RUSAGE_SELF, &m_usage) < 0) {
				fprintf(stderr, "ERROR: Could not get rusage.\n");
				exit(errno);
			}
			else {
				prev_user_time = (double)m_usage.ru_utime.tv_sec 
					       + (double)m_usage.ru_utime.tv_usec/MILLION;
				prev_sys_time = (double)m_usage.ru_stime.tv_sec 
					       + (double)m_usage.ru_stime.tv_usec/MILLION;
			}
		}

		switch (opt) {
			case 'a':
				f_crash();
				break;
			case 'c':
				f_command(argc, argv);				
				break;
			case 'd':
				f_default();
				break;
			case 'i':
				f_ignore();
				break;
			case 'l':
				f_close();
				break;
			case 'p':
				f_openpipe();
				break;
			case 'r':
				PROFILE_FLAG = 1;
				break;
			case 't':
				f_catch();
				break;
			case 'u':
				if (VERBOSE_FLAG)
					printf("--pause\n");
				pause();
				if (PROFILE_FLAG)
					f_profile(RUSAGE_SELF);
				break;
			case 'v':
				VERBOSE_FLAG = 1;
				break;
			case 'w':
				f_wait();
				break;
			case O_APPEND:
			case O_CLOEXEC:
			case O_CREAT:
			case O_DIRECTORY:
			case O_DSYNC:
			case O_EXCL:
			case O_NOFOLLOW:
			case O_NONBLOCK:
			case O_SYNC:
			case O_TRUNC:
				FILE_FLAG |= opt;
				if (VERBOSE_FLAG)
					printf("%s ", argv[optind-1]);
				break;
			case O_RDONLY:
			case O_WRONLY:
			case O_RDWR:
				FILE_FLAG |= opt;
				f_openfile();
				break;
		}
	}

	int return_val = 0;
	int status;
    	for (int i = 0; i < NUM_PID; i++) {
        	waitpid(pid_array[i], &status, 0);
        	int exit_status = WEXITSTATUS(status);
        
    		/*calculate the largest exit status*/
    		if(exit_status > return_val)
        		return_val = exit_status;
    	}

	/* clean up */
	for (int i = 0; i < NUM_FILES; i++)
		if (fd_array[i] != -1)
			if (close(fd_array[i]) < 0) {
				fprintf(stderr, "ERROR: Could not close fd%d\n", fd_array[i]);
				exit(EXIT_FAILURE);
			}

	free(fd_array);
	free(pid_array);
	free(pipe_array);

	free(process_array);
	if (PROFILE_FLAG) {
		if (getrusage(RUSAGE_SELF, &m_usage) < 0) {
				fprintf(stderr, "ERROR: Could not get rusage.\n");
				exit(errno);
		}
		else {
			printf("TOTAL RESOURCE USAGE\n");
			printf("\tTotal user time: \t\t%f s\n", *total_user_time);
			printf("\tTotal system time: \t\t%f s\n", *total_sys_time);			
			/*printf("\tTotal user time: \t\t%ld.%06ld s\n", 
				m_usage.ru_utime.tv_sec,
				m_usage.ru_utime.tv_usec);
			printf("\tTotal system time: \t\t%ld.%06ld s\n", 
				m_usage.ru_stime.tv_sec,
				m_usage.ru_stime.tv_usec);
			*/			
			printf("\tSoft page faults: \t\t%ld\n", m_usage.ru_minflt);
			printf("\tHard page faults: \t\t%ld\n", m_usage.ru_majflt);
			printf("\tVoluntary context switches: \t%ld\n", m_usage.ru_nvcsw);
			printf("\tInvoluntary context switches: \t%ld\n\n", m_usage.ru_nivcsw);
		}
	}
	return return_val;
}
