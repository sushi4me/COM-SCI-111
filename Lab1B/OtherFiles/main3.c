#define _GNU_SOURCE
#include <stdio.h>     
#include <stdlib.h>    
#include <getopt.h>   
#include <unistd.h>    
#include <fcntl.h>     
#include <sys/types.h> 
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include "catch.h"
#include "option.h"
#include "verbose.h"

void print_usage_parent(char* option, struct rusage reference)
{
  struct rusage usage;
  if(getrusage(RUSAGE_SELF, &usage)<0)
    {
      fprintf(stderr, "Fail to use getrusage()\n");
      exit(errno);
    }
  
  printf ("Resource usage for option %s\n", option);  
  double usrtime = (double)((usage.ru_utime.tv_sec-reference.ru_utime.tv_sec)  + (usage.ru_utime.tv_usec - reference.ru_utime.tv_usec)/1000000.0);
  double systime = (double)((usage.ru_stime.tv_sec-reference.ru_utime.tv_sec)  + (usage.ru_stime.tv_usec - reference.ru_stime.tv_usec)/1000000.0);
  printf("User time: %f\n", usrtime);
  printf("System Time: %f\n", systime);
  printf("\n");
}


void print_usage_child()
{
  struct rusage usage;
  if(getrusage(RUSAGE_CHILDREN, &usage)<0)
    {
      fprintf(stderr, "Fail to use getrusage()\n");
      exit(errno);
    }
  double usrtime = (double)(usage.ru_utime.tv_sec + usage.ru_utime.tv_usec/1000000.0);
  double systime = (double)(usage.ru_stime.tv_sec + usage.ru_stime.tv_usec/1000000.0);
  printf("\n");
  printf("Resource usage for all the children\n");
  printf("User time: %f\n", usrtime);
  printf("System time: %f\n", systime);
  printf("\n");
}

typedef struct{
   pid_t p_pid;
   int start;//find the starting index in argv
   int end;//find the ending index in argv
}process;

process* proc_array;
int* files;

int
main(int argc, char * argv[])
{
  int option = 0;
  int oflags = 0;
  int pipefd[2];
  files = (int*) malloc(0); //declare a dynamic array that map to the file descriptor
  int fsize = 0;
  int fd;
  int index;
  proc_array = (process*) malloc(0);
  int numchild = 0;
  int max_status = 0;
  struct rusage reference;
  while (option != -1) {
    if(verbose_flag && argv[optind]!='\0')
      {print_verbose(argc, argv, optind);}
    //do profile after an option is executed. We could do it before the option is updated
    option = getopt_long(argc, argv, "",long_options, NULL);
    switch (option) {
    case 3: /* rdonly */
      {
	if(profile_flag)
	  {
	    if(getrusage(RUSAGE_SELF, &reference)<0)
	      {
		fprintf(stderr, "Fail to call getrusage.\n");
		exit(errno);
	      }
	  }
	oflags = oflags | O_RDONLY;
	   if(optarg[0]=='-' && optarg[1] == '-')
	     {
	       fprintf(stderr, "rdonly has no argument\n");
	     }
	   else if (argv[optind]!='\0' && argv[optind][0] != '-' && argv[optind][1] != '-')//check whether there is more than one argument 
	     {
	       fprintf(stderr, "We should not put more than one argument in rdonly\n");
	     }
	   fd = open(optarg, oflags);
	    if(fd == -1)
	     {
	       fprintf(stderr, "No such file\n");
	     }
	   fsize++;
	   files = realloc(files, sizeof(int) * fsize);
	   if(files== NULL)
	   {
	       fprintf(stderr, "Fail to realloc the files table\n");
	   }
	   files[fsize-1] = fd;
	   oflags = 0;
	   if(profile_flag)
	     print_usage_parent("rdonly", reference);
	   break;
      }
  case 4: /* wronly */
      {
	if(profile_flag)
	  {
	    if(getrusage(RUSAGE_SELF, &reference)<0)
	      {
		fprintf(stderr, "Fail to call getrusage\n");
		exit(errno);
	      }
	  }
	oflags = oflags | O_WRONLY;
	if(optarg[0]=='-' && optarg[1]=='-')
	  {
	    fprintf(stderr, "wronly option has no argument\n");
	  }
	else if(argv[optind] != '\0' && argv[optind][0]!='-'&&argv[optind][1]!='-')
	  {
	    fprintf(stderr,"There should not be more than one argument in wronly\n");
	  }
       fd = open(optarg, oflags);
       if(fd == -1)
	{
	  fprintf(stderr, "No such file\n");
	}
       fsize++;
       files = realloc(files, sizeof(int) * fsize);
       if(files == NULL)
	{
	  fprintf(stderr, "Fail to realloc the files table\n");
	}
       files[fsize-1]= fd;
       oflags = 0;
       if(profile_flag)
	 print_usage_parent("wronly", reference);
       break;
       }
  case 6: /*read and write only*/
      {
	if(profile_flag)
	  {
	    if(getrusage(RUSAGE_SELF, &reference)<0)
	      {
		fprintf(stderr, "Fail to do getrusage\n");
		exit(errno);
	      }
	  }
	oflags = oflags | O_RDWR;
	if(optarg[0]=='-' && optarg[1] == '-')
	     {
	       fprintf(stderr, "rdwr has no argument\n");
	     }
	else if (argv[optind]!='\0' && argv[optind][0] != '-' && argv[optind][1] != '-')//check whether there is more than one argument
	     {
	       fprintf(stderr, "We should not put more than one argument in rdwr\n");
	     }
	fd = open(optarg, oflags);
	if(fd == -1)
	{
	  fprintf(stderr, "No such file\n");
	}
	fsize++;
	files = realloc(files, sizeof(int) * fsize);
	if(files == NULL)
	{
	  fprintf(stderr, "Fail to realloc the files table\n");
	}
	files[fsize-1] = fd;
	oflags = 0;
	if(profile_flag)
	  print_usage_parent("rdwr", reference);
	break; 
      }

  case 7:/*pipe*/
      {
	if(profile_flag)
	  {
	    if(getrusage(RUSAGE_SELF, &reference)<0)
	      {
		fprintf(stderr, "Fail to execute getrusage\n");
		exit(errno);
	      }
	  }
        files = realloc(files, sizeof(int)*(fsize+2));
	if(files == NULL)
	{
	   fprintf(stderr, "Fail to realloc the files table\n");
	}
	fsize+=2;
	if(pipe(pipefd)==-1) //allocate space on pipes
	  {
	    fprintf(stderr, "Fail to create a pipe");
	  }
	files[fsize-2]=pipefd[0];
	files[fsize-1]=pipefd[1];
	if(profile_flag)
	  print_usage_parent("pipe", reference);
	break;
      }
  case 5: /* command */
      {
	if(profile_flag)
	  {
	    if(getrusage(RUSAGE_SELF, &reference)<0)
	      {
		fprintf(stderr, "Fail to execute getrusage\n");
		exit(errno);
	      }
	  }
	proc_array = realloc(proc_array,(numchild+1)*sizeof(process)); 
	index  = optind-1;
	int filenum;
	filenum = atoi(argv[index]);
	int sd_in = files[filenum];
	if(fcntl(sd_in, F_GETFL) == -1 || filenum < 0|| filenum >= fsize)
	  {
	    //printf("I hate Stephen Curry\n");
	    fprintf(stderr, "Bad file descriptor\n");
	  }
     
	index++;
	filenum = atoi(argv[index]);
	int sd_out = files[filenum];;
	if(fcntl(sd_out, F_GETFL)==-1 || filenum < 0 || filenum >= fsize)
	  {
	    fprintf(stderr, "Bad file descriptor\n");
	  }
	index++;

	filenum = atoi(argv[index]);
	int sd_err = files[filenum];
	if(fcntl(sd_err, F_GETFL) == -1 || filenum < 0 || filenum >= fsize)
	  {
	    fprintf(stderr, "Bad file descriptor\n");
	  }

	int startindex = index+1;
	while(index < argc)
	  {
	    if(argv[index][0]=='-' && argv[index][1]=='-')
	      break;
	    index++;
	  }
	int endindex = index-1;
	proc_array[numchild].start = startindex;
	proc_array[numchild].end = endindex;
	//optind = endindex;
	int args_size = endindex - startindex +2;
	char** args = (char**) malloc( sizeof(char*)*(args_size));   
	char* cmd = (char*) malloc(sizeof(char)*strlen(argv[startindex]));
	strcpy(cmd, argv[startindex]);
	int i = 0;
	int j = startindex;
	while(j<= endindex)
	  {
	    args[i]= argv[j]; 
	    i++;
	    j++;
	  }
	args[i] = '\0';
        optind = endindex+1;
	pid_t pid = fork();
	if (pid == 0){
	  
	  if(dup2(sd_in,0)<0) /* replace with proper file descriptors */
	    {
	      fprintf(stderr, "Fail to execute dup2.\n");
	    }
	
	  if(dup2(sd_out,1)<0) // replace indexes of fd (0,1,2) with proper indexes
	    {
	      fprintf(stderr, "Fail to execute dup2\n");
	    }
	  if (dup2(sd_err,2)<0)
	    {
	      fprintf(stderr, "Fail to execute dup2\n");
	    }
	//close appropriate file descriptor entries. 
	  for(int i =0; i<fsize; i++)
	    {
	      close(files[i]);
	    }
	   close(pipefd[0]);
	   close(pipefd[1]);
	  if(execvp(cmd,args)<0){
	    fprintf(stderr, "Fail to execute the command using execvp\n");
	    exit(1);
	  };
       }
	else if (pid < 0)//error case
	  {
	    fprintf(stderr, "Fail to create a child process using fork\n");
	    exit(1);
	  }
	else //this runs the parent process, parent process should keet going
	  {
	    proc_array[numchild].p_pid = pid;
	    numchild++;
	    if(profile_flag)
	      print_usage_parent("command", reference);
	  }
	break;
    }
  case 8: //wait
      {
	if(profile_flag)
	  {
	    if(getrusage(RUSAGE_SELF, &reference)<0)
	      {
		fprintf(stderr, "Fail to execute getrusage\n");
		exit(errno);
	      }
	  }
	for(int i =0; i< fsize; i++)
	  close(files[i]);
	close(pipefd[0]);
	close(pipefd[1]);
	fsize=0;
	files = (int*)realloc(files, 0);
	for(int cd =0; cd< numchild; cd++)
	{
	  int status;
	  int exit_status;
          int return_pid = waitpid(-1, &status, 0);
	  if(return_pid == -1)
	    fprintf(stderr, "Fail to call waitpid\n");
	  if(WIFEXITED(status))//check whether the child terminated normally
	      exit_status = WEXITSTATUS(status);
	  if (exit_status > max_status)
	    max_status = exit_status;
	  printf("%d ", exit_status);
	  for (int j = 0; j< numchild; j++)
	  {
	    if(proc_array[j].p_pid == return_pid)
	    {
	      for (int k = proc_array[j].start; k <= proc_array[j].end; k++)
		 printf(" %s",argv[k]);
	      printf("\n");
	      break;
	    }
	  }
	}
	if(profile_flag)
	  {
	    print_usage_parent("wait", reference);
	    print_usage_child();
	  }
	break;
     }
  case 9: //close
      {
	if(profile_flag)
	  {
	    if(getrusage(RUSAGE_SELF, &reference)<0)
	      {
		fprintf(stderr, "Fail to execute getrusage\n");
		exit(errno);
	      }
	  }
	int closed_index = atoi(optarg);
	if(close(files[closed_index])<0)
	  {
	    fprintf(stderr, "Fail to close\n");
	    exit(errno);
	  }
	files[closed_index]=-1;
	if(profile_flag)
	  print_usage_parent("close", reference);
	break;
      }
  case 10: //abort
      {
	if(profile_flag)
	  {
	    if(getrusage(RUSAGE_SELF, &reference)<0)
	      {
		fprintf(stderr, "Fail to execute getrusage\n");
		exit(errno);
	      }
	  }
	raise(SIGSEGV); //implement an segmentation fault. This whole program will crash
	if(profile_flag)
	  print_usage_parent("abort", reference);
	break;
      }
  case 11: //catch
      {
	if(profile_flag)
	  {
	    if(getrusage(RUSAGE_SELF, &reference)<0)
	      {
		fprintf(stderr, "Fail to execute getrusage\n");
		exit(errno);
	      }
	  }
	if(signal(atoi(optarg),simpsh_handler_catch) == SIG_ERR){
	  fprintf(stderr, "Fail to catch signal %d\n", atoi(optarg));
	}
	if(profile_flag)
	  print_usage_parent("catch", reference);
	break;
      }
  case 12: //ignore
      {
	if(profile_flag)
	  {
	    if(getrusage(RUSAGE_SELF, &reference)<0)
	      {
		fprintf(stderr, "Fail to execute getrusage\n");
		exit(errno);
	      }
	  }
	if(signal(atoi(optarg),SIG_IGN) == SIG_ERR){
	  fprintf(stderr, "Fail to catch signal %d\n", atoi(optarg));
	}
	if(profile_flag)
	  print_usage_parent("ignore", reference);
	break;
      }
  case 13: //default
      {
	if(profile_flag)
	  {
	    if(getrusage(RUSAGE_SELF, &reference)<0)
	      {
		fprintf(stderr, "Fail to execute getrusage\n");
		exit(errno);
	      }
	  }
	if(signal(atoi(optarg),SIG_DFL) == SIG_ERR){
	  fprintf(stderr, "Fail to catch signal %d\n", atoi(optarg));
	}
	if(profile_flag)
	  print_usage_parent("default", reference);
	break;
      }
  case 14: //pause
      {
	if(profile_flag)
	  {
	    if(getrusage(RUSAGE_SELF, &reference)<0)
	      {
		fprintf(stderr, "Fail to execute getrusage\n");
		exit(errno);
	      }
	  }
	if(pause()<0)
	  fprintf(stderr, "Failure to pause\n");
	if(profile_flag)
	  print_usage_parent("pause", reference);
	break;
      }
  case 15: 
      oflags = oflags | O_APPEND;
      break;
  case 16:
      oflags = oflags | O_CLOEXEC;
      break;
  case 17:
      oflags = oflags | O_CREAT;
      break;
  case 18:
      oflags = oflags | O_DIRECTORY;
      break;
  case 19:
      oflags = oflags | O_DSYNC;
      break;
  case 20:
      oflags = oflags | O_EXCL;
      break;
  case 21:
      oflags = oflags | O_NOFOLLOW;
      break;
  case 22:
      oflags = oflags | O_NONBLOCK;
      break;
  case 23:
      oflags = oflags | O_RSYNC;
      break;
  case 24:
      oflags = oflags | O_SYNC;
      break;
  case 25:
      oflags = oflags | O_TRUNC;
      break;   
   }
  }
  free(files);
  free(proc_array);
  exit(max_status);
}
