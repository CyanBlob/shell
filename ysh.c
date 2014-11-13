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


extern int errno;

typedef void (*sighandler_t)(int);
//static 
char *my_argv[100], *my_envp[100];
//static 
char *search_path[10];

int argv_index = 0;

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

// copies envp into my_envp - Gary
void copy_envp(char **envp) //Experimentally, **my_envp and **envp are exactly the same, and neither one stores user variables, only system variables. -Andrew
{
    int index = 0;
    for(;envp[index] != NULL; index++) {
        my_envp[index] = (char *)
		malloc(sizeof(char) * (strlen(envp[index]) + 1));
        memcpy(my_envp[index], envp[index], strlen(envp[index]));
	//printf("ENVP: %s\n", envp[index]);
	//printf("MYVP: %s\n", my_envp[index]); 

    }
    //sleep(200);
}

// gets the envp that contains PATH and copies it to bin_path - Gary: could just return a pointer to the envp
void get_path_string(char **tmp_envp, char *bin_path)
{
    int count = 0;
    char *tmp;
    while(1) {
        tmp = strstr(tmp_envp[count], "PATH");
        if(tmp == NULL) {
            count++;
        } else {
            break;
        }
    }
        strncpy(bin_path, tmp, strlen(tmp));
}

// extracts individual paths from path_str and puts them in search_path[] - Gary
void insert_path_str_to_search(char *path_str) 
{
    int index=0;
    char *tmp = path_str;
    char ret[100];

    // advances to the = in the path - Gary
    while(*tmp != '=')
        tmp++;
    tmp++;

    // continue to the null terminator of the path string - Gary
    while(*tmp != '\0') {
    	// path is : delimited; if a delimiter is found add a "/" and a "\0" to ret and add ret to search_path[] - Gary
        if(*tmp == ':') {
            strncat(ret, "/", 1);
            search_path[index] = 
		(char *) malloc(sizeof(char) * (strlen(ret) + 1));
	    strncat(search_path[index], ret, strlen(ret));
            strncat(search_path[index], "\0", 1);
            index++;
            bzero(ret, 100);
        // else append the tmp char to ret - Gary
        } else {
            strncat(ret, tmp, 1);
        }
        tmp++;
    }
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

// executes the cmd with arguments in my_argv using execvp() - Gary
void call_execve(char *cmd)
{
    int i;
    printf("cmd is %s\n", cmd);
    // forks and then executes; exec replaces the child processe of the shell with the executed command process - Gary
    if(fork() == 0) {
        //i = execve(cmd, my_argv, my_envp);
	i = execvp(cmd, my_argv); //This fixed the 'echo' problem on my machine. execve(3) does not search for the command on the default PATH, but execvp does. I don't know if this will cause any problems down the line, as execvp(2) does not take the list of environment variables (my_envp) as an argument. -Andrew
        // if execvp() is successful the following doesn't run because this process has been replaced - Gary
        // if execvp() is unsuccessful then this process still exists and it prints an error (stored in errno automatically) - Gary
        printf("errno is %d\n", errno);
        if(i < 0) {
            printf("%s: %s\n", cmd, "command not found"); //This is the error message being printed from 'echo'. The error spawns from the value of 'i', which is assigned by the function 'execve(cmd, my_argv, my_envp); -Andrew
            exit(1);        
        }
    } else {
    	// the parent waits for the child process to complete - Gary
        wait(NULL);
    }
}


//This new function will print the output of a command into the file specified after the '>>' string from user input -Andrew
void call_execve_outredirect(char *cmd, int k)
{
    int i;
    char *new_argv[100]; 
    //filename gets set to 'my_argv[d+1], which is the next argument after 'ls' -Andrew
    char *filename = my_argv[k + 1];
    printf("%s\n", filename);

    //Copies everything before the '<<' from my_argv[] into new_argv[] -Andrew
    for (k = 0; k <= argv_index; k++)
    {
	    if(strcmp(my_argv[k], ">>") != 0)
	    {
		new_argv[k] = my_argv[k];
	    }
	    else if(strcmp(my_argv[k], ">>") == 0)
	    {
		while( k < 100)
		{
			new_argv[k] = NULL;
			k++;
		}
		break;
	    }
    } 

    printf("cmd is %s\n", cmd);
    if(fork() == 0) {


	//Opens a file to be written to with name 'filename'. Clears existing data in file, and will create the file if it does not exist. Also sets read/write permissions -Andrew    
	int file = open(filename, O_TRUNC | O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH);
	
	//Keeps track of 'stdout' so that we can go back to writing to the command line -Andrew
	int old_stdout = dup(1);

	int x = 0;
	for (x = 0; x < 100; x++)

	//Starts writing output to the file -Andrew
	dup2(file,1);


	//This gets passed new_argv[] instead of my_argv[] so that we don't get the '<<' and the filename included in what gets executed by execvp() -Andrew
	i = execvp(cmd, new_argv); //This fixed the 'echo' problem on my machine. execve(3) does not search for the command on the default PATH, but execvp does. I don't know if this will cause any problems down the line, as execvp(2) does not take the list of environment variables (my_envp) as an argument. -Andrew 
	//Resumes writing output to stdout -Andrew
	close(file);
	dup2(old_stdout,1);

	//printf("%d\n", i);
	printf("errno is %d\n", errno);
        if(i < 0) {
            printf("%s: %s\n", cmd, "command not found"); //This is the error message being printed from 'echo'. The error spawns from the value of 'i', which is assigned by the function 'execve(cmd, my_argv, my_envp); -Andrew
            exit(1);        
        }
    } else {
        wait(NULL);
    }
}

void call_execve_inredirect(char *cmd, int k) //int k is the index where the redirection symbol was found -Andrew
{
    int i = 0;
    char c;
    char oneword[100];
    char *new_argv[100];
    FILE *file;
    int BUFSIZE = 100;
    char buffer[100];
    //filename gets set to 'my_argv[d+1], which is the next argument after 'ls' -Andrew
    char *filename = my_argv[k + 1];
    printf("%s\n", filename);

    //Copies everything after the '>>' from my_argv[] into new_argv[] -Andrew
    
    file = fopen(filename, "r");
    for (k = 0; k <= argv_index; k++)
    {
	    if(strcmp(my_argv[k], "<<") != 0)
	    {
		new_argv[k] = my_argv[k];
	    }
	    else if(strcmp(my_argv[k], "<<") == 0)
	    {


		/*
		printf("TEST1\n");	
		while (fgets(buffer,100,file))
		{

			printf("%s", buffer);
			printf("TEST2\n");
			strcpy(new_argv[k],"This is a\0");

			printf("TEST3\n");
			printf("Line %d: %s",k,new_argv[k]);
			k++;
		}
		getchar();*/



		/*do
		{
		    c = fscanf(file, "%s" , oneword);
		    buffer = oneword;
		    //strcpy(new_argv[k],oneword);
		    printf("%s %s %d\n",oneword, buffer, k);
		    k++;
		} while (c != EOF);*/

		//Copies the contents of the input file to the end of new_argv[] -Andrew
		

		
		new_argv[k] = malloc(BUFSIZE);

		while (fgets(new_argv[k], BUFSIZE, file)) 
		{
		        k++;
		        new_argv[k] = malloc(BUFSIZE);
			strcat(new_argv[k - 1], "");
			printf("TEST\n");
			printf("TEST: %d %s\n",k, new_argv[k - 1]);
		} 
		
	
		//fclose(file);

		//Sets the remainder of new_argv[] to NULL -Andrew
		while( k < 100)
		{
			new_argv[k] = NULL;
			k++;
		}
		break;
	    }
    } 

    printf("cmd is %s\n", cmd);
    if(fork() == 0) {

	//This gets passed new_argv[] instead of my_argv[] so that we don't get the '<<' and the filename included in what gets executed by execvp() -Andrew
	    
	i = execvp(cmd, new_argv); //This fixed the 'echo' problem on my machine. execve(3) does not search for the command on the default PATH, but execvp does. I don't know if this will cause any problems down the line, as execvp(2) does not take the list of environment variables (my_envp) as an argument. -Andrew 


	for(k = 0; k < 100; k++)
		free(new_argv[k]);

	printf("errno is %d\n", errno);
        if(i < 0) {
            printf("%s: %s\n", cmd, "command not found"); //This is the error message being printed from 'echo'. The error spawns from the value of 'i', which is assigned by the function 'execve(cmd, my_argv, my_envp); -Andrew
            exit(1);        
        }
    } else {
        wait(NULL);
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
int main(int argc, char *argv[], char *envp[]) //envp is an array that stores the users environment variables. -Andrew
{
    char c;
    int i, fd;
    char *tmp = (char *)malloc(sizeof(char) * 100);
    char *path_str = (char *)malloc(sizeof(char) * 256);
    char *cmd = (char *)malloc(sizeof(char) * 100);
    
    // ignore terminal interrupt signals - Gary: seems redundant given the next line
    signal(SIGINT, SIG_IGN);
    // handle terminal interrupt signals with the function handle_signal - Gary
    signal(SIGINT, handle_signal);

    // copies envp into my_envp - Gary: Why? Couldn't envp be used everywhere my_envp is used?
    copy_envp(envp);

    // copies the variable in envp that contains PATH to path_str - Gary
    get_path_string(my_envp, path_str);
    // extracts individual paths from path_str and adds them to the search_path[] array - Gary
    insert_path_str_to_search(path_str);

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
	char outputredirect[] = ">>";
	char inputredirect[] = "<<";
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
                       if(index(cmd, '/') == NULL) {
                       	   // adds a path to cmd if a valid one is found in search_paths[] - Gary: attach_path always == 0
                           if(attach_path(cmd) == 0) {
                           	   // iterates through the argv and compares them to ouputredirect '<<' set t = 1 if found - Gary
				   for (d = 0; d < argv_index; d++)
			           {
				      if(strcmp(my_argv[d], outputredirect) == 0)
				      {
			 	          t = 1;
			  	          break;
			              }
				      else if(strcmp(my_argv[d], inputredirect) == 0)
				      {
				          t = 2;
					  break;
				      }

				   }
                               // if output redirect wasn't found - Gary
			       if(t == 0)
                                   call_execve(cmd);
                               // if output redirect was found - Gary
			       else if (t == 1)
				   call_execve_outredirect(cmd, d);
			       else if (t == 2)
			           call_execve_inredirect(cmd, d);
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
                               call_execve(cmd);
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
