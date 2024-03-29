/* Author(s): Spencer Green.
 *
 * This is lab9.c the csc60mshell
 * This program serves as a skeleton for doing labs 9, 10, 11.
 * Student is required to use this program to build a mini shell
 * using the specification as documented in direction.
 * Date: Fall 2018
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#define MAXLINE 80
#define MAXARGS 20
#define MAX_PATH_LENGTH 50
#define TRUE 1

/* function prototypes */
int parseline(char *cmdline, char **argv);
void process_input(int argc, char **argv); 
void handle_redir(int count, char *argv[]); 

/* ----------------------------------------------------------------- */
/*                  The main program starts here                     */
/* ----------------------------------------------------------------- */

int main(void)
{
    char cmdline[MAXLINE];
    char *argv[MAXARGS];
    int i;
    int argc ;
    int status;
    pid_t pid;


    /* Loop forever to wait and process commands */
    while (TRUE) {
	/* Print your shell name: csc60mshell (m for mini shell) */
	printf("csc60mshell> ");
    
	/* Read the command line */
	fgets(cmdline, MAXLINE, stdin);

        /* Call parseline to build argc/argv */
        argc =  parseline(cmdline, argv);
 
        /* If nothing was entered, continue */
        if (argc == 0) {
        continue;
        }

       /* Exit command*/
        if(strcmp(argv[0], "exit") ==0){
        exit(EXIT_SUCCESS);
        }

        /*pring total arguments  */
        printf("%s%d%s\n","You entered ", argc, " arguments");
        
        /*prit arguments with for loop*/
        for(i=0;i<=argc;i+=1) {
        printf("%s%i%s%s\n","the argument[",i,"] is: ",argv[i]);
        }
        
        /* Print Working Directory*/
        if(strcmp(argv[0], "pwd") ==0) {
        char buf[MAX_PATH_LENGTH];
        getcwd(buf, MAX_PATH_LENGTH);
        printf("%s\n", buf);
        continue;       
        }

        /* Change Directory*/
        else if(strcmp(argv[0], "cd")==0){
        char *dir;
           if(argc == 1){
           dir = getenv("HOME");
           }
           else {
             dir = argv[1];
           }
        chdir(dir);
        if(dir<0){
        fprintf(stderr, "Problem with Directory change.\n");
        exit(EXIT_FAILURE);
        }
        continue;
        }
 
       
        /* If user hits enter key without a command, continue to loop */
	/* again at the beginning */
	/*  Hint: if argc is zero, no command declared */
	/*  Hint: look up for the keyword "continue" in C */

	/* Handle build-in command: exit, pwd, or cd  */
	/* Put the rest of your code here */


	/* Else, fork off a process */
      else {
	    pid = fork();
          switch(pid)
    	    {
	    	case -1:
		    perror("Shell Program fork error");
	            exit(EXIT_FAILURE);
	   	case 0:
		    /* I am child process. I will execute the command, */
		    /* and call: execvp */
		    process_input(argc, argv);
		    break;
	   	default:
		    /* I am parent process */
		    if (wait(&status) == -1)
		    	perror("Parent Process error");
		    else
		   	printf("Child returned status: %d\n",status);
		    break;
	    } 	/* end of the switch */

	}	/* end of the if-else-if */
    }		/* end of the while */
} 		/* end of main */

/* ----------------------------------------------------------------- */
/*                  parseline                                        */
/* ----------------------------------------------------------------- */
/* parse input line into argc/argv format */

int parseline(char *cmdline, char **argv)
{
    int count = 0;
    char *separator = " \n\t"; /* Includes space, Enter, Tab */
 
    /* strtok searches for the characters listed in separator */
    argv[count] = strtok(cmdline, separator);

    while ((argv[count] != NULL) && (count+1 < MAXARGS)) 
    	argv[++count] = strtok((char *) 0, separator);
     		
    return count;  //This becomes "argc" back in main.
}
/* ----------------------------------------------------------------- */
/*                  process_input                                    */
/* ----------------------------------------------------------------- */
void process_input(int argc, char **argv) {                       
    
    int return_value;
    /* Step 1: Call handle_redir to deal with operators:            */
    /* < , or  >, or both                                           */
    handle_redir(argc, argv);

    /* Step 2: perform system call execvp to execute command        */
    /* Hint: Please be sure to review execvp.c sample program       */
    return_value=execvp(argv[0], argv); 
     if (return_value == -1) {                                        
        fprintf(stderr, "Error on the exec call\n");              
        _exit(EXIT_FAILURE);                                      
     }                                                            
 
 }
/* ----------------------------------------------------------------- */
void handle_redir(int count, char *argv[]){
    int out_redir = 0;
    int in_redir = 0;
    int i;
    int fd;
 
    for(i=0; i < count; i+=1){
       if(strcmp(argv[i],">") == 0){  //strcmp function
            if(out_redir != 0 ){
                 fprintf(stderr, "Cannot input from more than one file.\n");
                 _exit(EXIT_FAILURE);
            } else if (i == 0){
                 fprintf(stderr,"No command entered.\n");
                 _exit(EXIT_FAILURE);
            }
            out_redir = i;
       }else if(strcmp(argv[i],"<") ==0){
            if(in_redir != 0){
                 fprintf(stderr,"Cannot input from more than one file. \n");
                 _exit(EXIT_FAILURE);
            } else if( i == 0){
                 fprintf(stderr, "No command entered.\n");
                 _exit(EXIT_FAILURE);
            }
            in_redir = i;
       }
    }

    if(out_redir != 0){
	    if(argv[out_redir + 1]== NULL){
		    fprintf(stderr,"ERROR: There is no File. \n" );
              _exit(EXIT_FAILURE);
         }

         fd =  open(argv[out_redir+1],O_RDWR|O_CREAT|O_TRUNC, S_IRUSR|S_IWUSR);
         if (fd == -1){
              perror("Error opening file. \n");
                  
         }
         dup2(fd,1 );
         close(fd);
         argv[out_redir] = NULL;
         
    }
 
    if(in_redir != 0){
         if(argv[in_redir+1]== NULL){
              fprintf(stderr,"ERROR: There is no File\n" );
              _exit(EXIT_FAILURE);
         }
         
         fd = open(argv[in_redir+1], O_RDONLY);
         if (fd == -1){
              perror("Error opening file.\n");
              _exit(EXIT_FAILURE);
         }
         dup2(fd,0);
         close(fd);
         argv[in_redir] = NULL;
         }
    
}
/* ----------------------------------------------------------------- */


/* ----------------------------------------------------------------- */
