// CS 111 Lab 1

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <wait.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <signal.h>

int verbose_flag;
int profile_flag;
int wait_flag;
int* pipes;
int* files;
int n_files;
int max_files;
int* pids;
int n_pids;
int max_pids;
int flags;

struct ProcessInfo
{
	pid_t pid;
	char **args;
};
int total_process_info;
int n_process_info;
struct ProcessInfo *process_info;

void
print_usage(int who)
{
	struct rusage usage;
	if (getrusage(who, &usage) < 0) {
		fprintf(stderr, "error with getrusage()\n");
		exit(errno);
	}
	printf("\tUser time used:\t\t\t%lld seconds\n", (long long) usage.ru_utime.tv_sec);
	printf("\t\t\t\t\t%lld microseconds\n", (long long) usage.ru_utime.tv_usec);
	printf("\tSystem time used:\t\t%lld seconds\n", (long long) usage.ru_stime.tv_sec);
	printf("\t\t\t\t\t%lld microseconds\n", (long long) usage.ru_stime.tv_usec);
	printf("\tMax resident set size:\t\t%ld\n", usage.ru_maxrss);
	printf("\tSoft page faults:\t\t%ld\n", usage.ru_minflt);
	printf("\tHard page faults:\t\t%ld\n", usage.ru_majflt);
	printf("\tVoluntary context switches:\t%ld\n", usage.ru_nvcsw);
	printf("\tInvoluntary context switches:\t%ld\n", usage.ru_nivcsw);
}

void
check_file_array()
{
	// If files is maxed out, reallocate it
	if (n_files == max_files) {
		max_files *= 2;
		files = realloc(files, max_files);
		pipes = realloc(pipes, max_files);
		if (files == NULL || pipes == NULL) {
			fprintf(stderr, "error reallocating space for files\n");
			exit(errno);
		}
	}
	return;
}

void
open_pipe() {
	int pipeEnds[2];
	if (pipe(pipeEnds) < 0) {
		fprintf(stderr, "error opening pipe\n");
		exit(errno);
	}
	pipes[n_files] = 1;
	pipes[n_files + 1] = 0;
	for (int i = 0; i < 2; i++) {
		files[n_files++] = pipeEnds[i];
		check_file_array();
	}
}

int
is_pipe(int num)
{
	if (pipes[num] < 0 || pipes[num] >= n_files) return 0;
	if (pipes[num] == 1 || (num > 0 && pipes[num - 1] == 1)) return 1;
	return 0;
}

int
other_pipe_end(int num)
{
	return pipes[num] == 1 ? num + 1 : num - 1;
}

void
run_command(char* cmd_args[])
{
	// Save the original STDERR_FILENO for future reference
	int saved_stderr = dup(STDERR_FILENO);
	if (saved_stderr < 0) {
		fprintf(stderr, "error with dup\n");
		exit(errno);
	}

	// Redirect std file descriptors to files
	if (dup2(files[atoi(cmd_args[0])], STDIN_FILENO) < 0 ||
		dup2(files[atoi(cmd_args[1])], STDOUT_FILENO) < 0 ||
		dup2(files[atoi(cmd_args[2])], STDERR_FILENO) < 0) {
		dup2(saved_stderr, STDERR_FILENO);
		close(saved_stderr);
		fprintf(stderr, "error with dup2\n");
		exit(errno);
	}

	for (int i = 0; i < 3; i++) {
		int fd_num = atoi(cmd_args[i]);
		if (is_pipe(fd_num) && close(files[other_pipe_end(fd_num)]) < 0) {
			fprintf(stderr, "error while closing FD%i", other_pipe_end(fd_num));
			exit(errno);
		} else if (files[fd_num] != -1 && close(files[fd_num]) < 0) {
			fprintf(stderr, "ERROR: error while closing FD%i", fd_num);
			exit(errno);
		}
		else
			files[fd_num] = -1;
	}

	// Excecute the command
	execvp(cmd_args[3], &cmd_args[3]);
	_exit(errno);
}



char**
get_command_args(int argc, char* argv[])
{
	int index = optind - 1;
	int command_args_size = 15;
	int n_command_args = 0;

	// Allocate space for the command arguments and check for errors
	char** command_args = (char**) malloc(command_args_size * sizeof(char*));
	if (command_args == NULL) {
		fprintf(stderr, "error allocating space for command_args\n");
		exit(errno);
	}

	// Add arguments to command_args until we reach the next option
	while (1) {
		if (index == argc ||
				(argv[index][0] == '-' && argv[index][1] == '-')) {
			optind = index;
			command_args[n_command_args] = NULL;
			break;
		}

		command_args[n_command_args++] = argv[index++];
		printf("%s ", command_args[n_command_args-1]);

		// If needed, reallocate space for command_args and check for errors
		if (n_command_args == command_args_size) {
			command_args_size *= 2;
			command_args = (char**) realloc(command_args, command_args_size);
			if (command_args == NULL) {
				fprintf(stderr, "error reallocating command_args\n");
				exit(errno);
			}
		}
	}

	// Make sure the user supplied enough arguments to the command
	if (n_command_args < 4) {
		fprintf(stderr, "error: command requires at least 4 arguments\n");
		return NULL;
	}

	// Check to make sure that all of the requested descriptors exist
	for (int i = 0; i < 3; i++) {
		int file_des_index = atoi(command_args[i]);
		if (file_des_index < 0 || file_des_index >= n_files) {
			fprintf(stderr, "error: bad file descriptor\n");
			return NULL;
		}
	}

	return command_args;
}

void
open_file()
{
	// If verbose, print out the action
	if (verbose_flag == 1)
		printf("--%sonly %s\n", ((flags == O_RDONLY) ? "rd" : "wr"), optarg);

	// Check if file exists
	if ((flags & O_CREAT) != O_CREAT && access(optarg, F_OK) < 0) {
		fprintf(stderr, "error: specified file %s doesn't exist\n", optarg);
		exit(EXIT_FAILURE);
	}

	// Open the file and check for errors
	/* printf("DEBUG: opening %s with flags 0x%X\n", optarg, flags); */
	files[n_files] = open(optarg, flags, S_IRUSR | S_IWUSR | S_IROTH);
	pipes[n_files] = 0;
	if (files[n_files] < 0) {
		fprintf(stderr, "error opening file %s\n", optarg);
		exit(errno);
	}
	flags = 0;

	n_files++;

	check_file_array();
	return;
}

void
command_option(int argc, char* argv[])
{
	// Get the command arguments
	char** command_args = get_command_args(argc, argv);
	if (command_args == NULL) return;

	// If verbose, print out the action
	if (verbose_flag) {
		printf("--command");
		int k = 0;
		for (char* c = command_args[k]; c != NULL; c = command_args[++k]) {
			printf(" %s", c);
		}
		printf("\n");
	}

	// Fork and run the command
	int pid = fork();
	if (pid == 0) {
		run_command(command_args);
	}
	else if (pid < 0) {
		fprintf(stderr, "error with fork\n");
		exit(errno);
	}
	else {
		if (n_process_info == total_process_info) {
			total_process_info *= 2;
			process_info = (struct ProcessInfo *)
				realloc(process_info, sizeof (struct ProcessInfo));
			if (!process_info) {
				fprintf(stderr, "error with realloc\n");
				exit(errno);
			}
		}
		pids[n_pids++] = pid;
		process_info[n_process_info].pid = pid;

		int n_args = 0;
		int args_size = 15;
		process_info[n_process_info].args = (char **)
			malloc(args_size * sizeof (char *));
		if (!process_info[n_process_info].args) {
			fprintf(stderr, "error with malloc\n");
			exit(errno);
		}

		int k = 0;
		for (char *c = command_args[k+3];
				c != NULL; c = command_args[++k+3]) {
			if (n_args == args_size) {
				args_size *= 2;
				process_info[n_process_info].args =	(char **)
					realloc(process_info[n_process_info].args,
							args_size * sizeof (char *));
				if (!process_info[n_process_info].args) {
					fprintf(stderr, "error with realloc\n");
					exit(errno);
				}
			}
			process_info[n_process_info].args[k] = c;
			n_args++;
        }
		process_info[n_process_info].args[k] = NULL;

		free(command_args);
		n_process_info++;
	}
}

void crash()
{
	int* p = NULL;
	*p = 1;
}

void catch_signal(int signo)
{
	fprintf(stderr, "%d caught\n", signo);
	exit(signo);
}

void print_subcommands()
{
	int status;
	int exit_status;

	for (int i = 0; i < n_process_info; i++) {
		exit_status = waitpid(-1, &status, 0);
		printf("%d ", status);
		int j;
		for (j = 0; j < n_process_info; j++)
			if (exit_status == process_info[j].pid)
				break;
		int k = 0;
		for (char* c = process_info[j].args[k];
				c != NULL; c = process_info[j].args[++k])
			printf("%s ", c);
		printf("\n");
	}
}

int
main(int argc, char* argv[])
{
	max_files = 100 * sizeof(int);
	max_pids = 100 * sizeof(int);
	n_files = 0;
	n_pids = 0;
	verbose_flag = 0;
	profile_flag = 0;
	wait_flag = 0;
	flags = 0;
	n_process_info = 0;
	total_process_info = 15;
	process_info = (struct ProcessInfo *)
		malloc(total_process_info * sizeof (struct ProcessInfo));
	if (!process_info) {
		fprintf(stderr, "error with malloc\n");
		exit(errno);
	}
	struct option long_options[] = {
		{"cloexec",		no_argument,		0,	O_CLOEXEC},
		{"directory",	no_argument,		0,	O_DIRECTORY},
		{"dsync",		no_argument,		0,	O_DSYNC},
		{"nofollow",	no_argument,		0,	O_NOFOLLOW},
		{"rsync",		no_argument,		0,	O_RSYNC},
		{"append",		no_argument,		0,	O_APPEND},
		{"creat",		no_argument,		0,	O_CREAT},
		{"excl",		no_argument,		0,	O_EXCL},
		{"nonblock",	no_argument,		0,	O_NONBLOCK},
		{"sync",		no_argument,		0,	O_SYNC},
		{"trunc",		no_argument,		0,	O_TRUNC},
		{"rdonly",		required_argument,	0,	O_RDONLY},
		{"wronly",		required_argument,	0,	O_WRONLY},
		{"rdwr",		required_argument,	0,	O_RDWR},
		{"pipe",		no_argument,		0,	'p'},
		{"command",		required_argument,	0,	'c'},
		{"verbose",		no_argument,		0,	'v'},
		{"close",		required_argument, 	0, 	'l'}, // for now
		{"abort",		no_argument, 		0, 	'b'},
		{"catch",		required_argument, 	0, 	'h'},
		{"ignore",		required_argument,	0,	'i'},
		{"default",		required_argument,	0,	'd'},
		{"pause",		no_argument,		0,	's'},
		{"profile",		no_argument,		0,	'f'},
		{"wait",		no_argument,		0,	'w'},
		{0,				0,					0,	0}
	};

	// Allocate space for file descriptors and PIDs and check for errors
	files = malloc(max_files);
	pipes = malloc(max_files);
	if (files == NULL || pipes == NULL) {
		fprintf(stderr, "error allocating space for files\n");
		exit(errno);
	}
	pids = malloc(max_pids);
	if (pids == NULL) {
		fprintf(stderr, "error allocating space for PIDs\n");
		exit(errno);
	}

	// Parse the options
	while (1) {
		int option_index = 0;
		int c =	getopt_long(argc, argv, "", long_options, &option_index);
		if (c < 0) break; // Reached the end of argv
		int signum;
		switch (c) {
			case O_APPEND:
			case O_CLOEXEC:
			case O_DIRECTORY:
			case O_DSYNC:
			case O_NOFOLLOW:
			case O_CREAT:
			case O_EXCL:
			case O_NONBLOCK:
			case O_SYNC:
			case O_TRUNC:
				if (verbose_flag)
					printf("%s ", argv[optind - 1]);
				flags |= c;
				break;
			case O_RDONLY:
			case O_RDWR:
			case O_WRONLY: // open a file
				flags |= c;
				open_file();
				if (profile_flag) print_usage(RUSAGE_SELF);
				break;
			case 'p': // open a pipe
				if (verbose_flag)
					printf("--pipe\n");
				open_pipe();
				if (profile_flag) print_usage(RUSAGE_SELF);
				break;
			case 'c': // execute a command
				command_option(argc, argv);
				if (profile_flag) print_usage(RUSAGE_SELF);
				break;
			case 'v': // Set the verbose flag
				verbose_flag = 1;
				break;
			case 'f': // Set the profile flag
				if (verbose_flag)
					printf("--profile\n");
				profile_flag = 1;
				break;
			case 'l': // close
				if (verbose_flag)
					printf("--close %i\n", atoi(optarg));
				if (atoi(optarg) < 0 || atoi(optarg) >= n_files) {
					fprintf(stderr, "this file does not exit\n");
					exit(EXIT_FAILURE);
				}
				if (close(files[atoi(optarg)]) < 0) {
					fprintf(stderr, "error with close\n");
					exit(errno);
				}
				if (profile_flag) print_usage(RUSAGE_SELF);
				break;
			case 'b': // abort
				crash();
				break;
			case 'i': // ignore
				signum = atoi(optarg);
				if (verbose_flag)
					printf("--ignore %d\n", signum);
				if (signal(signum, SIG_IGN) == SIG_ERR) {
					fprintf(stderr, "error with signal\n");
					exit(errno);
				}
				if (profile_flag) print_usage(RUSAGE_SELF);
				break;
			case 'd': // default
				signum = atoi(optarg);
				if (verbose_flag)
					printf("--default %d\n", signum);
				if (signal(signum, SIG_DFL) == SIG_ERR) {
					fprintf(stderr, "error with signal\n");
					exit(errno);
				}
				if (profile_flag) print_usage(RUSAGE_SELF);
				break;
			case 's': // pause
				pause();
				if (profile_flag) print_usage(RUSAGE_SELF);
				break;
			case 'h': // catch
				signum = atoi(optarg);
				if (verbose_flag)
					printf("--catch %d\n", signum);
				if (signal(signum, catch_signal) == SIG_ERR) {
					fprintf(stderr, "error with signal\n");
					exit(errno);
				}
				if (profile_flag) print_usage(RUSAGE_SELF);
				break;
			case 'w':
				if (verbose_flag)
					printf("--wait\n");
				print_subcommands();
				if (profile_flag) {
					printf("Parent Usage Information:\n");
					print_usage(RUSAGE_SELF);
					printf("Children Usage Information:\n");
					print_usage(RUSAGE_CHILDREN);
				}
				break;
			case ':':
			case '?':
			default:
				exit(EXIT_FAILURE);
		}
	}

	// Find the max exit status from the subcommands
	int exit_max = 0;
	int exit_val;
	for (int i = 0; i < n_pids; i++) {
		int status;
		waitpid(pids[i], &status, 0);
		if (WIFEXITED(status)) {
			exit_val = WEXITSTATUS(status);
			exit_max = (exit_val > exit_max) ? exit_val : exit_max;
		}
	}

	// Close open file descriptors
	for (int i = 0; i < n_files; i++) close(files[i]);
	free(files); // Deallocate file array
	free(pids); // Deallocate PID array
	free(pipes);

	for (int i = 0; i < n_process_info; i++)
		free(process_info[i].args);
	free(process_info);

	exit(exit_max);
}
