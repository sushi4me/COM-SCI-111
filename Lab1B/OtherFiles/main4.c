//main.c
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <ucontext.h>
#include <sys/time.h>
#include <sys/resource.h>

//defines
#define TRUE 1
#define FALSE 0
#define cmd_size 100

/*Global Variables*/

/*flags*/
static int verbose_flag;
static int wait_flag;

/*temporary storage*/
static int pid_child[1000];
static int fd[1000];
static int fd_pipe[1000];
static int pipe_fd[2];
static int close_fd;
static int catch_sig;
static int ignore_sig;
static int default_sig;
static int cmd_fd[3];
char *cmd[cmd_size];
static int return_val;
const mode_t mode = 0644;

struct pid_info{
    pid_t p_pid;
    char *p_args[cmd_size];
};

struct pid_info wait_infos[100];

/*Helper Functions*/
int isAnOption(char *cstring){
    if((cstring[0] != '-') || (cstring[1] != '-'))
        return FALSE;
    else
        return TRUE;
}

void flushCmd(){
    for(int i = 0; i < cmd_size; i++)
        cmd[i] = NULL;
}

int isNumber(char *string){
    int i;
    for(i = 0; string != NULL && *(string + i) != '\0'; i++) {
        if (!isdigit(*(string+i)))
            return FALSE;
        return TRUE;
    }
}

// Signal Handlers
void catch_handler(int sig){
    fprintf(stderr, "error: signal %d caught\n", sig);
    exit(sig);
}

void ignore_handler(int sig, siginfo_t *si, void *arg){
    signal(sig, SIG_IGN);
    ucontext_t *context = (ucontext_t*)arg;
    context->uc_mcontext.gregs[REG_RIP]++;
}

void pause_handler(int sig, siginfo_t *si, void *arg){
    fprintf(stderr, "error: fail to pause\n");
    exit(EXIT_FAILURE);
}



//main function
int main(int argc, char **argv){

    static struct option long_opts[] = 
    {
        /*file flags options*/
        {"append",      no_argument, 0,    'a'},
        {"cloexec",     no_argument, 0,    'l'},
        {"creat",       no_argument, 0,    'C'},
        {"directory",   no_argument, 0,    'd'},
        {"dsync",       no_argument, 0,    'D'},
        {"excl",        no_argument, 0,    'e'},
        {"nofollow",    no_argument, 0,    'n'},
        {"nonblock",    no_argument, 0,    'N'},
        {"rsync",       no_argument, 0,    's'},
        {"sync",        no_argument, 0,    'S'},
        {"trunc",       no_argument, 0,    't'},
        
        /*file opening options*/
        {"rdonly",      required_argument,  0,  'r'},
        {"wronly",      required_argument,  0,  'w'},
        {"rdwr",        required_argument,  0,  'R'},
        {"pipe",        no_argument,        0,  'p'},

        /*subcommand options*/
        {"command",     required_argument,  0,          'c'},
        {"wait",        no_argument,        0,          'W'},
        
        /*miscellaneous options*/
        {"close",       required_argument,  0,  'L'},
        {"verbose",     no_argument,        &verbose_flag, 1},
        {"abort",       no_argument,        0,  'A'},
        {"catch",       required_argument,  0,  'T'},
        {"ignore",      required_argument,  0,  'i'},
        {"default",     required_argument,  0,  'f'},
        {"pause",       no_argument,        0,  'P'},
        {0,             0,                  0,  0}
    };
    
    int long_opts_ind;
    int curr_opt;
    int curr_optind;
    int next_optind;
    int oflags = 0;
    char *option;
    
    int pid_child_ind = 0;
    int fd_ind = 0;
    int cmd_ind = 0;
    int pid_info_ind = 0;

    verbose_flag = 0;
    wait_flag = 0;
    ignore_sig = 0;
    default_sig = 0;

    return_val = 0;

    pid_t pid;
    pid_t c_pid;
    
    //signal handling
    struct sigaction sa;
    
    curr_optind = optind;
    curr_opt = getopt_long(argc, argv, "", long_opts, &long_opts_ind);
    next_optind = optind;




    if(curr_opt == -1){
        fprintf(stderr, "error: no options found\n");
    }
    else {
        if(!isAnOption(argv[curr_optind])){
            fprintf(stderr, "error: argument found before options and was ignored\n");
        }
        do {
            switch(curr_opt){
                // append
                case 'a':{
                if((next_optind != argc) && !isAnOption(argv[next_optind])){
                    fprintf(stderr, "error: append can not accept any arguments, all arguments to append were ignored\n");
                    }
                    oflags |= O_APPEND;
                    if(verbose_flag){
                        printf("--append\n");
                    }
                    break;
                }
                // cloexec
                case 'l':{
                    if((next_optind != argc) && !isAnOption(argv[next_optind])){
                        fprintf(stderr, "error: cloexec can not accept any arguments, all arguments to cloexec were ignored\n");
                    }
                    oflags |= O_CLOEXEC;
                    if(verbose_flag){
                        printf("--cloexec\n");
                    }
                    break;
                }
                // creat
                case 'C':{
                    if((next_optind != argc) && !isAnOption(argv[next_optind])){
                        fprintf(stderr, "error: creat can not accept any arguments, all arguments to creat were ignored\n");
                        exit(EXIT_FAILURE);
                    }
                    oflags |= O_CREAT;
                    if(verbose_flag){
                        printf("--creat\n");
                    }
                    break;
                }
                // directory
                case 'd':{
                    if((next_optind != argc) && !isAnOption(argv[next_optind])){
                        fprintf(stderr, "error: directory can not accept any arguments, all arguments to directory were ignored\n");
                    }
                    oflags |= O_DIRECTORY;
                    if(verbose_flag){
                        printf("--directory\n");
                    }
                    break;
                }
                // dsync
                case 'D':{
                    if((next_optind != argc) && !isAnOption(argv[next_optind])){
                        fprintf(stderr, "error: dsync can not accept any arguments, all arguments to dsync were ignored\n");
                    }
                    oflags |= O_DSYNC;
                    if(verbose_flag){
                        printf("--dsync\n");
                    }
                    break;
                }
                // excl
                case 'e':{
                    if((next_optind != argc) && !isAnOption(argv[next_optind])){
                        fprintf(stderr, "error: excl can not accept any arguments, all arguments to excl were ignored\n");
                    }
                    oflags |= O_EXCL;
                    if(verbose_flag){
                        printf("--excl\n");
                    }
                    break;
                }
                // nofollow
                case 'n':{
                    if((next_optind != argc) && !isAnOption(argv[next_optind])){
                        fprintf(stderr, "error: nofollow can not accept any arguments, all arguments to nofollow were ignored\n");
                    }
                    oflags |= O_NOFOLLOW;
                    if(verbose_flag){
                        printf("--nofllow\n");
                    }
                    break;
                }
                // nonblock
                case 'N':{
                    if((next_optind != argc) && !isAnOption(argv[next_optind])){
                        fprintf(stderr, "error: nonblock can not accept any arguments, all arguments to nonblock were ignored\n");
                    }
                    oflags |= O_NONBLOCK;
                    if(verbose_flag){
                        printf("--nonblock\n");
                    }
                    break;
                }
                // rsync
                case 's':{
                    if((next_optind != argc) && !isAnOption(argv[next_optind])){
                        fprintf(stderr, "error: rsync can not accept any arguments, all arguments to rsync were ignored\n");
                    }
                    oflags |= O_RSYNC;
                    if(verbose_flag){
                        printf("--rsync\n");
                    }
                    break;
                }
                // sync
                case 'S':{
                    if((next_optind != argc) && !isAnOption(argv[next_optind])){
                        fprintf(stderr, "error: sync can not accept any arguments, all arguments to sync were ignored\n");
                    }
                    oflags |= O_SYNC;
                    if(verbose_flag){
                        printf("--sync\n");
                    }
                    break;
                }
                // trunc
                case 't':{
                    if((next_optind != argc) && !isAnOption(argv[next_optind])){
                        fprintf(stderr, "error: trunc can not accept any arguments, all arguments to trunc were ignored\n");
                    }
                    oflags |= O_TRUNC;
                    if(verbose_flag){
                        printf("--trunc\n");
                    }
                    break;
                }
                // rdonly, rdwr, wronly
                case 'r':
                case 'R':
                case 'w':
                    if (curr_opt == 'r'){
                        oflags |= O_RDONLY;
                        option = "rdonly";
                    }
                    else if (curr_opt == 'R'){
                        oflags |= O_RDWR;
                        option = "rdwr";
                    }
                    else if (curr_opt == 'w'){
                        oflags |= O_WRONLY;
                        option = "wronly";
                    }
                    if((next_optind != argc) && !isAnOption(argv[next_optind])){
                        fprintf(stderr, "error: %s can only accept one argument, further arguments to %s were ignored\n", option, option);
                    }
                    if(verbose_flag){
                        printf("--%s %s\n", option, optarg);
                    }
                    int temp_fd = open(optarg, oflags, mode);
                    if (errno == EEXIST){
                        fprintf(stderr, "error: file \"%s\" already exists\n", optarg);
                        exit(errno);
                    }
                    if (errno == ENOTDIR){
                        fprintf(stderr, "error: file \"%s\" is a non-direcotry file\n", optarg);
                        exit(errno);
                    }
                    if (temp_fd == -1){
                        fprintf(stderr, "error: could not open file \"%s\"\n", optarg);
                        exit(errno);
                    }
                    fd[fd_ind++] = temp_fd;
                    fd_pipe[fd_ind] = 0;
                    
                    
                    
                    /*clean oflags*/
                    oflags = 0;
                    
                    break;
                    
                // pipe
                case 'p':{
                    if((next_optind != argc) && !isAnOption(argv[next_optind])){
                        fprintf(stderr, "error: pipe can not accept any arguments, all arguments to pipe were ignored\n");
                    }
                    if(verbose_flag){
                        printf("--pipe\n");
                    }
                    if(pipe(pipe_fd) == -1){
                        fprintf(stderr, "error: failure to create a pipe\n");
                    }
                
                    /*identify pipe file descriptors to the pipe array*/
                    fd[fd_ind++] = pipe_fd[0];
                    fd_pipe[fd_ind] = 1;
                    fd[fd_ind++] = pipe_fd[1];
                    fd_pipe[fd_ind] = 1;
                    
                    break;
                }
        
                //command
                case 'c':{
                    while((optind != argc) && !isAnOption(argv[optind])){
                                optind++;
                    }
                    if(optind < (curr_optind + 5))
                        fprintf(stderr, "error: missing arguments to --command, your command will not be run\n");
                    else{
                        cmd_fd[0] = atoi(optarg);
                        cmd_fd[1] = atoi(argv[next_optind++]);
                        cmd_fd[2] = atoi(argv[next_optind++]);

                        cmd_ind = 0;
                        //for(int i = 0; i < 3; i++)
                        //  printf("cmd_fd[%d] = %d\n", i, cmd_fd[i]);
                        
                        flushCmd();
        
                        while((next_optind != argc) && !isAnOption(argv[next_optind])){
                            char *char_ptr = malloc(sizeof(argv[next_optind]));
                            char *p_args_ptr = malloc(sizeof(argv[next_optind]));
                            //printf("%d ", (int)sizeof(argv[next_optind]));
                            cmd[cmd_ind] = char_ptr;
                            wait_infos[pid_info_ind].p_args[cmd_ind] = p_args_ptr;
                            strcpy(cmd[cmd_ind], argv[next_optind]);
                            strcpy(wait_infos[pid_info_ind].p_args[cmd_ind], argv[next_optind]);
                            cmd_ind++;
                            next_optind++;
                        }
                        
                        if(verbose_flag){
                            printf("--command");
                            for(int i = 0; i < 3; i++){
                                printf(" %d", cmd_fd[i]);
                            }
                            for(int i = 0; i < cmd_size; i++){
                                if(cmd[i] != NULL){
                                    printf(" %s", cmd[i]);
                                }
                            }
                            printf("\n");
                        }
                                
                        c_pid = fork();
                        if(c_pid == 0){ 
                            
                            /*close unused side of the pipe*/
                            if (fd_pipe[cmd_fd[0]])
                                close(fd[cmd_fd[0]+1]);
                            if (fd_pipe[cmd_fd[1]])
                                close(fd[cmd_fd[1]-1]);
                            if (fd_pipe[cmd_fd[2]])
                                close(fd[cmd_fd[2]-1]);
                        
                            /*direct file descriptors*/
                            dup2(fd[cmd_fd[0]], STDIN_FILENO);
                            dup2(fd[cmd_fd[1]], STDOUT_FILENO);
                            dup2(fd[cmd_fd[2]], STDERR_FILENO);

                            /*execute command*/
                            if(execvp(cmd[0], cmd) == -1){
                                fprintf(stderr, "error: command failed\n");
                            }
                        }
                        else if(c_pid > 0){
                            
                            pid_child[pid_child_ind] = c_pid;
                            pid_child_ind++;
                            wait_infos[pid_info_ind].p_pid = c_pid;

                            /*close unused side of the pipe*/
                            if (fd_pipe[cmd_fd[0]]){
                                close(fd[cmd_fd[0]]);
                                fd[cmd_fd[0]] = -1;
                            }
                            if (fd_pipe[cmd_fd[1]]){
                                close(fd[cmd_fd[1]]);
                                fd[cmd_fd[1]] = -1;
                            }
                            if (fd_pipe[cmd_fd[2]]){
                                close(fd[cmd_fd[2]]);
                                fd[cmd_fd[2]] = -1;
                            }
                        }
                        else { //couldn't create child process
                            fprintf(stderr, "error: could not create child process\n");
                        }

                        for(int i = 0; i < cmd_size; i++)
                            free(cmd[i]);
                    }
                    
                    pid_info_ind++;
                    
                    break;
                }

                //wait
                case 'W':{
                    if (verbose_flag)
                        printf("--wait\n");
                    
                    int status;
                    int exit_status;

                    for (int i = 0; i < pid_info_ind; i++) {
                        exit_status = waitpid(-1, &status, 0);
                        printf("%d ", status);
                        
                        int j;
                        for (j = 0; j < pid_info_ind; j++)
                            if (exit_status == wait_infos[j].p_pid)
                                break;
                        int k = 0;
                        for (; wait_infos[j].p_args[k]!= NULL; k++)
                            printf("%s ", wait_infos[j].p_args[k]);
                        
                        printf("\n");
                    }
                    
                    break;
                }

                // close
                case 'L':{
                    if (verbose_flag)
                        printf("--close %s\n", optarg);
                    if (!isNumber(optarg)){
                        fprintf(stderr, "error: close requires an integer argument\n");
                        exit(EXIT_FAILURE);
                    }
                    close_fd = atoi(optarg);
                    if (close_fd > fd_ind){
                        fprintf(stderr, "error: the entered file descriptor number is invalid\n");
                        exit(EXIT_FAILURE);
                    }
                    close(fd[close_fd]);
                    fd[close_fd] = -1;
                    
                    break;
                }
                // abort
                case 'A':{
                    if (verbose_flag)
                        printf("--abort\n");
                    raise(SIGSEGV);
                    break;
                }
                // catch
                case 'T':{
                    if (verbose_flag) {
                        printf("--catch %s\n", optarg);
                    }
                    catch_sig = atoi(optarg);
                    if (!isNumber(optarg) || catch_sig < 0){
                        fprintf(stderr, "error: catch requires a valid integer argument\n");
                        continue;
                    }
                    sa.sa_handler = catch_handler;
                    sigemptyset(&sa.sa_mask);
                    sa.sa_flags = SA_SIGINFO;
                    
                    if (sigaction(catch_sig, &sa, NULL) < 0){
                        /*handle error*/
                        fprintf(stderr, "error: fail to handle signal catch %d.\n",catch_sig);
                        exit(EXIT_FAILURE);
                    }
                    
                    break;
                }
                // ignore
                case 'i':{
                    ignore_sig = atoi(optarg);
                    if (verbose_flag) {
                        printf("--ignore %s\n", optarg);
                    }
                    if (!isNumber(optarg) || ignore_sig < 0){
                        fprintf(stderr, "error: ignore requires a valid integer argument\n");
                        continue;
                    }
                    
                    sa.sa_sigaction = &ignore_handler;
                    sa.sa_flags = SA_SIGINFO;
                    
                    if (sigaction(ignore_sig, &sa, NULL) < 0){
                        /*handle error*/
                        fprintf(stderr, "error: fail to ignore signal %d.\n",ignore_sig);
                        exit(EXIT_FAILURE);
                    }
                    
                    break;
                }
                // default
                case 'f':{
                    default_sig = atoi(optarg);
                    if (verbose_flag) {
                        printf("--default %s\n", optarg);
                    }
                    if (!isNumber(optarg) || default_sig < 0){
                        fprintf(stderr, "error: default requires a valid integer argument\n");
                        continue;
                    }
                    if (signal(default_sig, SIG_DFL) < 0){
                        /* Handle error */
                        fprintf(stderr, "error: fail to handle signal %d with default\n", default_sig);
                        exit(EXIT_FAILURE);
                        break;
                    }
                                      
                    break;
                }
                // pause
                case 'P':{
                    if (verbose_flag) {
                        printf("--pause %s\n", optarg);
                    }
                    pause();
                    
                    break;
                }
                case '?':{
                    fprintf(stderr, "error: option not recognized or no option argument found\n");
                    break;
                }
            }

            //move to next option
            while((optind != argc) && !isAnOption(argv[optind]))
                optind++;
            curr_optind = optind;
            curr_opt = getopt_long(argc, argv, "", long_opts, &long_opts_ind);
            next_optind = optind;
        } while(curr_opt != -1);
    }

    /*close all file descriptors that were opened*/
    for(int i = 0; i < fd_ind; i++){
        if (fd[i] != -1){
            if(close(fd[i]) != 0){
                fprintf(stderr, "error: could not close file at logical index %d\n", i);
                exit(EXIT_FAILURE);
            }
        }
    }
    
    int status;
    for (int i = 0; i < pid_child_ind; i++){
        waitpid(pid_child[i], &status, 0);
        int exit_status = WEXITSTATUS(status);
        
    /*calculate the largest exit status*/
    if(exit_status > return_val)
        return_val = exit_status;
    }
    
    /*free the memory allocated for --wait commnands and arguments*/
    for (int i = 0; i < pid_info_ind; i++){
        for (int j = 0; j < cmd_size; j++)
            free(wait_infos[i].p_args[j]);
    }

    return return_val;
}
