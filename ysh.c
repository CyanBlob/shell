#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>

extern int errno;

typedef void (*sighandler_t)(int);
//static 
char *my_argv[100], *my_envp[100];
//static 
char *search_path[10];

int argv_index = 0;

float usage[1440] = {-1};
int counter = 0;
FILE *cpufile;
char cpu [5];
float cpu_float;
float cpu_avg = 0;

void get_cpu_usage()
{
	int x;
	while(1 == 1)
	{
		cpufile = fopen("/proc/loadavg", "r");
		fscanf(cpufile, "%s%*[^\n]", cpu);

		
		fclose(cpufile);		
		
		cpu_float = atof(cpu);

		usage[counter % 1440] = cpu_float;


		//printf("usage[%d]
		cpu_avg = 0; 
		for (x = 0; x <= counter % 1440; x++)
		{
			cpu_avg = cpu_avg + usage[x];
			//printf("%d, %2.2f\n", x, cpu_avg);
			//sleep(5);
		}
		if (counter < 1440)		
			cpu_avg = cpu_avg / ((counter % 1440) + 1);
		else 
			cpu_avg = (cpu_avg / 1440);

		counter++;
		sleep(60);
	}

}


// when a terminal interrupt signal is received... - Gary
void handle_signal(int signo)
{
    // ... print the prompt - Gary
    printf("\n[MY_SHELL ] ");
    // ... flush standard output - Gary: I think stdout is unbuffered by default, so this is unneccessary
    fflush(stdout);
}

// splits tmp_argv into arguments and stores them in my_argv - Gary
void fill_argv(char *tmp_argv)
{
    argv_index = 0;
    // copying pointer tmp_argv to pointer foo - Gary: intentional obfuscation?
    char *foo = tmp_argv;
    //int index = 0;
    char ret[100];
    bzero(ret, 100);
    // iterates through tmp_argv (foo) until null terminator - Gary
    while(*foo != '\0') {
    	// arguments capped at 10 for some reason - Gary
        if(argv_index == 10)
            break;

        if(*foo == ' ') {
            if(my_argv[argv_index] == NULL)
                my_argv[argv_index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
            else {
                bzero(my_argv[argv_index], strlen(my_argv[argv_index]));
            }
            strncpy(my_argv[argv_index], ret, strlen(ret));
            strncat(my_argv[argv_index], "\0", 1);
            bzero(ret, 100);
            argv_index++;
        } else {
            strncat(ret, foo, 1);
        }
        foo++;
        /*printf("foo is %c\n", *foo);*/
    }
    my_argv[argv_index] = (char *)malloc(sizeof(char) * strlen(ret) + 1);
    strncpy(my_argv[argv_index], ret, strlen(ret));
    strncat(my_argv[argv_index], "\0", 1);
}

// adds a path to cmd if it finds a valid path to cmd in search_path[] - Gary: appears to always return 0
int attach_path(char *cmd)
{
    char ret[100];
    int index;
    int fd;
    bzero(ret, 100);
    // iterates through the paths in search_path[] - Gary
    for(index=0;search_path[index]!=NULL;index++) {
    	// adds the cmd to each search_path - Gary
        strcpy(ret, search_path[index]);
        strncat(ret, cmd, strlen(cmd));
        // attempts to open the cmd at the search path, if it finds one that works it adds the path to the cmd - Gary
        if((fd = open(ret, O_RDONLY)) > 0) {
            strncpy(cmd, ret, strlen(ret));
            close(fd);
            return 0;
        }
    }
    return 0;
}

// execute command in my_argv[0]
void call_execvp() {
 if(fork() == 0) {
  execvp(my_argv[0], my_argv);
  printf("command not found: %s\n", my_argv[0]);
  exit(1);
 }
 else wait(NULL);
}

// input redirection
void call_execvp_inredirect(int d) {
 if(fork() == 0) {
  int file = open(my_argv[d+1], O_RDONLY);
  dup2(file, 0);
  my_argv[d] = NULL;
  execvp(my_argv[0], my_argv);
  printf("command not found: %s\n", my_argv[0]);
  exit(1);
 }
 else wait(NULL);
}

// output redirection
void call_execvp_outredirect(int d) {
 if (fork() == 0) {
  my_argv[d] = NULL;
  int file = open(my_argv[d+1], O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);
  int old_out = dup(1);
  dup2(file, 1);
  execvp(my_argv[0], my_argv);
  dup2(old_out, 1);
  printf("command not found: %s\n", my_argv[0]);
  exit(1);
 }
 else wait(NULL);
}

// piping process
void call_execvp_pipe_process(int d) {
 if(fork() == 0) {
  my_argv[d] = NULL;
  int file = open("pipebuffer.tmp", O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);
  int old_out = dup(1);
  dup2(file, 1);
  execvp(my_argv[0], my_argv);
  dup2(old_out, 1);
  printf("command not found: %s\n", my_argv[0]);
  exit(1);
 }
 else wait(NULL);

 if (fork() == 0) {
  int file = open("pipebuffer.tmp", O_RDONLY );
  dup2(file, 0);
  execvp(my_argv[d+1], &my_argv[d+1]);
  printf("command not found: %s\n", my_argv[d+1]);
  exit(1);
 }
 else wait(NULL);
}

// Background Process - Shashi
// Create a background process that outputs CPU usage information for every 10 seconds
// Background Process - Shashi
void background_process(char* cmd, int k)
{
    
    int i;
    pid_t child_process_id = 0;
    pid_t sid = 0;
    char *filename = my_argv[k + 1];
    char *line[100];
    int argv_size;

    for (i=0;i<100;i++)
    {
        if(i < k){
            argv_size = strlen(my_argv[i]);
            line[i] = malloc(argv_size*sizeof(char));
            strcpy(line[i], my_argv[i]);
        }
        else
            line[i] = NULL;
    }

    if (filename  == NULL)
    {
        filename = "BackgroundLog.txt";
    }


    int file = open(filename, O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH); 
    printf("cmd is %s\n", cmd);

    // create child process and check for failure
    if ((child_process_id = fork()) < 0)
    {
        printf("fork failed while creating background process!\n");
        // Return failure in exit status
        return;
    }
    else if (child_process_id > 0)
    {
        printf("process_id of background process %d \n", child_process_id);
        // return success in exit status
    }
    //process id if the user wants to kill the background process
    
    //parent process exits
    if((sid = setsid()) < 0){return;}


    //output to file
    dup2(file,1);
    //execute the command
    i = execvp(cmd, line);
    printf("errno is %d\n", errno);
    if(i < 0) {
        printf("%s: %s\n", cmd, "command not found"); //This is the error message being printed from 'echo'. The error spawns from the value of 'i', which is assigned by the function 'execve(cmd, my_argv, my_envp); -Andrew
        exit(1);
    exit(0);
    }
}

// clears my_argv[] - Gary
void free_argv()
{
    int index;
    for(index=0;my_argv[index]!=NULL;index++) {
        bzero(my_argv[index], strlen(my_argv[index])+1);
        my_argv[index] = NULL;
        free(my_argv[index]);
    }
}

// main - Gary
int main(int argc, char *argv[], char *envp[]) { //envp is an array that stores the users environment variables. -Andrew
 char c;
 int i, fd;
 char *tmp = (char *)malloc(sizeof(char) * 100);
 char *path_str = (char *)malloc(sizeof(char) * 256);
 char *cmd = (char *)malloc(sizeof(char) * 100);

 pthread_t xtid;
 pthread_create(&xtid, NULL, (void* (*) (void*)) get_cpu_usage, NULL);

 // ignore terminal interrupt signals - Gary: seems redundant given the next line
 // handle terminal interrupt signals with the function handle_signal - Gary
 signal(SIGINT, SIG_IGN);
 signal(SIGINT, handle_signal);

 // copies envp into my_envp - Gary: Why? Couldn't envp be used everywhere my_envp is used?
 for (i = 0; envp[i] != NULL; i++) {
  my_envp[i] = (char*) malloc(sizeof(char) * (strlen(envp[i]) + 1));
  memcpy(my_envp[i], envp[i], strlen(envp[i]));
 }

 // copies the variable in envp that contains PATH to path_str - Gary
 char* tmp2 = NULL;
 for (i = 0; tmp2 == NULL; i++) tmp2 = strstr(my_envp[i], "PATH");
 strncpy(path_str, tmp2, strlen(tmp2));

 // extracts individual paths from path_str and adds them to the search_path[] array - Gary
 char ret[100];
 char* tmp3 = path_str;
 for (tmp3 = path_str; *tmp3 != '='; tmp3++);
 tmp3++;
 i = 0;
 for (tmp3++; *tmp3 != '\0'; tmp3++) {
  if (*tmp == ':') {
   strncat(ret, "/", 1);
   search_path[i] = (char*) malloc(sizeof(char) * (strlen(ret) + 1));
   strncat(search_path[i], ret, strlen(ret));
   strncat(search_path[i], "\0", 1);
   i++;
   bzero(ret, 100);
  }
  else strncat(ret, tmp3, 1);
 }

    // fork into two processes and ... - Gary : This seems like a complicated way to clear the screen.
    // ... in the child processes run clear and exit the child process - Gary
    if(fork() == 0) {
        execve("/usr/bin/clear", argv, my_envp);
        exit(1);
    // ... in the parent process wait for the child to exit - Gary
    } else {
        wait(NULL);
    }
    
    // print the my_shell prompt and unnecessarily flush stdout... again - Gary
    printf("[MY_SHELL ] ");
    fflush(stdout);


    //In order to get redirection working, we need to look at the dup2 system call -Andrew
    // c = getchar(); loops until c is EOF - Gary: EOF never happens
    while(c != EOF) {
	int d = 0;
	int t = 0;
	
	char cpuredirect[] = "cpu";
	char piperedirect[]= "|";
	char outputredirect[] = ">";
	char inputredirect[] = "<";
    	char backgroundredirect[] = "&";
        c = getchar();
        // switch on character from getchar() - Gary
        // adds every c to tmp until it gets to newline - Gary
        switch(c) {
            // case: newline/enter - Gary: a massive switch statement with only one case in it
            // if at the null terminator in temp, just reprint the my_shell prompt - Gary
            case '\n': if(tmp[0] == '\0') {
                       printf("[MY_SHELL ] ");
                   // if not at null terminator in tmp then ...- Gary 
                   } else {
                       // split tmp into arguments and store them in my_argv - Gary
                       fill_argv(tmp);
                       // copy the first argv into cmd and print it - Gary
		       strncpy(cmd, my_argv[0], strlen(my_argv[0]));
		       printf("CMD: %s\n", cmd);
                       strncat(cmd, "\0", 1);
                       // if there are no forwards slashes in the cmd ... - Gary
                       if(index(cmd, '/') == NULL || strcmp(cmd, "./SuperBash") == 0) {
                       	   // adds a path to cmd if a valid one is found in search_paths[] - Gary: attach_path always == 0
                           if(attach_path(cmd) == 0) {
                           	   // iterates through the argv and compares them to ouputredirect '<<' set t = 1 if found - Gary
				   for (d = 0; d <= argv_index; d++)
			           {
					printf("my_argv[d] = %s\n", my_argv[d]);
				      if(strcmp(my_argv[d], outputredirect) == 0)
				      {
			 	          //printf("Found >>\n");
					  t = 1;
			  	          break;
			              }
				      else if(strcmp(my_argv[d], inputredirect) == 0)
				      {
				          t = 2;
					  break;
				      }
				      else if(strcmp(my_argv[d], piperedirect) == 0){
					  t = 3;
					  break;
				      }
				      else if(strcmp(my_argv[d], cpuredirect) == 0)
				      {
					  //printf("Found ??\n");
					  t = 4;
					  break;
				      }
				      else if(strcmp(my_argv[d], backgroundredirect) == 0)
                      		      {
                        		  t = 5;
                        		  break;
                		      }
					
				   }
                               // if output redirect wasn't found - Gary
			       if(t == 0)
                                   call_execvp();
                               // if output redirect was found - Gary
			       else if (t == 1)
				   call_execvp_outredirect(d);
			       else if (t == 2)
			           call_execvp_inredirect(d);
			       else if (t == 3)
			           call_execvp_pipe_process(d);
			       else if ( t == 4)
				   printf("Your current cpu usage is:\n1 minute average: %2.2f\n24 hour average: %2.2f\n", cpu_float, cpu_avg);
				else if( t == 5)
                    		    background_process(cmd, d);	
                    		    
			       t = 0;
			       //printf("d = %d\n",d);
			   // if attach_path is not equal to 0 - Gary: attach_path is always 0
                           } else {
                               printf("%s: command not found\n", cmd);
                           }
                       // if there was a forward slash in the cmd ... - Gary
                       } else {
                       	   // try and open the cmd and if successful exec it - Gary
                           if((fd = open(cmd, O_RDONLY)) > 0) {
                               close(fd);
                               call_execvp();
                           // if you can't open it then command not found, print error - Gary
                           } else {
                               printf("%s: command not found\n", cmd);
                           }
                       }
                       // clear my_argv[], reprint shell prompt, clear cmd - Gary
                       free_argv();
                       printf("[MY_SHELL ] ");
                       bzero(cmd, 100);
                   }
                   // clear tmp - Gary
                   bzero(tmp, 100);
                   break;
            // default case: concat c to tmp - Gary
            default: strncat(tmp, &c, 1);
                 break;
        }
    }
    free(tmp);
    free(path_str);
    for(i=0;my_envp[i]!=NULL;i++)
        free(my_envp[i]);
    for(i=0;i<10;i++)
        free(search_path[i]);
    printf("\n");
    return 0;
}
