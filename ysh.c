/*
 Group #: 16
 Members: George Davis
          Shashi Dongur
          Gary Johnson
          Andrew Thomas
 Class:   CSCE3600
 Session: Fall 2014
 Section: T/Th 10:00am
 Project: Build a Unix Shell
 Compile: gcc ysh.c - lpthread -o YSH
 Usage:   see README.md
 Files:   ysh.c          - this source file
          YSH            - compiled executable
          SuperBash.c    - source for SuperBash/Bash++ utility
          SuperBash      - compiled SuperBash/Bash++ utility
          pipebuffer.tmp - temporay buffer used by piping command
          background.log - output file used by background processes
          README.md      - usage documentation
*/

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


char* my_argv[100] = { NULL };


// cpu usage globals
float usage[1440] = {-1};
int counter = 0;
FILE *cpufile;
char cpu [5];
float cpu_float;
float cpu_avg = 0;


// gets cpu usage
void get_cpu_usage() {
 int x;
 while(1 == 1) {
  cpufile = fopen("/proc/loadavg", "r");
  fscanf(cpufile, "%s%*[^\n]", cpu);
  fclose(cpufile);
  cpu_float = atof(cpu);
  usage[counter % 1440] = cpu_float;
  cpu_avg = 0;

  for (x = 0; x <= counter % 1440; x++)	{
   cpu_avg = cpu_avg + usage[x];
  }

  if (counter < 1440) cpu_avg = cpu_avg / ((counter % 1440) + 1);
  else	cpu_avg = (cpu_avg / 1440);

  counter++;
  sleep(60);
 }
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


// runs a process in the background
void call_execvp_background_process(int d) {
 pid_t child_pid = fork();

 if (child_pid == 0) {
  my_argv[d] = NULL;
  char* filename = my_argv[d + 1];
  if (filename == NULL) filename = "background.log";
  int file = open(filename, O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);
  dup2(file, 1);
  execvp(my_argv[0], my_argv);
  printf("command not found: %s\n", my_argv[0]);
  exit(1);
 }
 else printf("process_id of background process %d \n", child_pid);
}


// main
int main(int argc, char *argv[]) {
 char c;
 int i;
 char *tmp = (char *)malloc(sizeof(char) * 100);
 char *cmd = (char *)malloc(sizeof(char) * 100);

 // old command variables
 int ran_once = 0;

 // cpu usage thread generation
 pthread_t xtid;
 pthread_create(&xtid, NULL, (void* (*) (void*)) get_cpu_usage, NULL);

 // ignore signals from zombie children (thus automatically reaping them)
 signal(SIGCHLD, SIG_IGN);

 // clear the screen
 if(fork() == 0) execvp("clear", argv);
 else wait(NULL);

 // print the my_shell prompt
 printf("shell> ");

 // c = getchar(); loops until c is EOF
 while(c != EOF) {
  int d = 0;
  int t = 0;
  char cpuredirect[] = "cpu";
  char piperedirect[]= "|";
  char outputredirect[] = ">";
  char inputredirect[] = "<";
  char backgroundredirect[] = "&";
  c = getchar();
  // switch on character from getchar()
  // adds every c to tmp until it gets to newline
  // case: newline/enter - Gary: a massive switch statement with only one case in it
  // if at the null terminator in temp, just reprint the my_shell prompt
  if (c == '\n') {
   if(tmp[0] == '\0') printf("shell> ");
   // if not at null terminator in tmp then ...
   else {
    // split tmp into arguments and store them in my_argv
    if (!ran_once || !(tmp[0] == '!' && tmp[1] == '!')) {
     // copy tmp to cmd
     strncpy(cmd, tmp, 100);

     // tokenize cmd and store the index in my_argv
     int i = 0;
     my_argv[i] = strtok(cmd, " ");
     while (my_argv[i] != NULL) {
      i++;
      my_argv[i] = strtok(NULL, " ");
     }

     // set ran once
     ran_once = 1;
    }

    // iterates through the argv and compares them to ouputredirect '<<' set t = 1 if found
    for (d = 0; my_argv[d] != NULL; d++) {
     if(strcmp(my_argv[d], outputredirect) == 0) {
      t = 1;
      break;
     }
     else if(strcmp(my_argv[d], inputredirect) == 0) {
      t = 2;
      break;
     }
     else if(strcmp(my_argv[d], piperedirect) == 0) {
      t = 3;
      break;
     }
     else if(strcmp(my_argv[d], cpuredirect) == 0) {
      t = 4;
      break;
     }
     else if(strcmp(my_argv[d], backgroundredirect) == 0) {
      t = 5;
      break;
     }
    }

    if (t == 0) call_execvp();
    else if (t == 1) call_execvp_outredirect(d);
    else if (t == 2) call_execvp_inredirect(d);
    else if (t == 3) call_execvp_pipe_process(d);
    else if (t == 4) printf("Your current cpu usage is:\n1 minute average: %2.2f\n24 hour average: %2.2f\n", cpu_float, cpu_avg);
    else if (t == 5) call_execvp_background_process(d);
    t = 0;

    // clear my_argv[], reprint shell prompt, clear cmd
    printf("shell> ");
   }
   // clear tmp
   bzero(tmp, 100);
  }
  // default case: concat c to tmp
  else strncat(tmp, &c, 1);
 }
 printf("\n");
 return 0;
}
