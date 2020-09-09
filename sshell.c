/*
 *  This is a simple shell program from
 *  rik0.altervista.org/snippetss/csimpleshell.html
 *  It's been modified a bit and comments were added.
 *
 *  But it doesn't allow misdirection, e.g., <, >, >>, or |
 *  The project is to fix this.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <sys/types.h>
#define BUFFER_SIZE 80
#define ARR_SIZE 80

// #define DEBUG 1  /* In case you want debug messages */
//this is where you manipulate the parsing of commands
void parse_args(char *buffer, char** args, 
                size_t args_size, size_t *nargs)
{
    char *buf_args[args_size]; 
    
    char *wbuf = buffer;

    buf_args[0]=buffer; 

    args[0]=buffer;  /* First argument */
/*
 *  The following replaces delimiting characters with '\0'. 
 *  Example:  " Aloha World\n" becomes "\0Aloha\0World\0\0"
 *  Note that the for-loop stops when it finds a '\0' or it
 *  reaches the end of the buffer.
 */   
    for(char **cp=buf_args; (*cp=strsep(&wbuf, " \n\t")) != NULL ;){
        if ((**cp != '\0') && (++cp >= &buf_args[args_size]))
            break; 
    }

/* Copy 'buf_args' into 'args' */    
    size_t j=0;
    for (size_t i=0; buf_args[i]!=NULL; i++){ 
        if(strlen(buf_args[i])>0)  /* Store only non-empty tokens */
            args[j++]=buf_args[i];
    }
    
    *nargs=j;
    args[j]=NULL;
}
struct pipe
{
    //file descriptor 
    int pipefd[2];

};

int main(int argc, char *argv[], char *envp[]){
    char buffer[BUFFER_SIZE];
    char *args[ARR_SIZE];

    size_t num_args;
    pid_t pid;
    
    while(1){
        printf("ee468>> "); 
        fgets(buffer, BUFFER_SIZE, stdin); /* Read in command line */
        //this part parses the command
        parse_args(buffer, args, ARR_SIZE, &num_args);
        //if a valid command is sent 
        if (num_args>0) {

            //if exit is entered the program exits
            if (!strcmp(args[0], "exit" )) exit(0); 
            int pipeCount = 0;
            int pipeLocations[num_args/2];
            // goes through the loop and count the pipes and record their locations
            for(int i = 0; args[i]; i++){
                if(!strcmp(args[i], "|" )){
                    pipeCount++;
                    pipeLocations[pipeCount] = i; 
                }
            }
            // instantiate the pipes
#ifdef DEBUG
            int createdpipes
#endif
            if(pipeCount){
                struct pipe pipefd[pipeCount];
                for(int i=0; i <= pipeCount; i++){
                    if(pipe(pipefd[i].pipefd)< 0) {
                        printf("error making pipe: %d \n", i);
                        perror("pipe");
                    }
#ifdef DEBUG
                    createdpipes++;
#endif
                }
            }
#ifdef DEBUG
            printf("making pipes: i made %d pipes\n", createdpipes);
#endif
            

            pid = fork();
            if (pid){  /* Parent */
#ifdef DEBUG
                printf("Waiting for child (%d)\n", pid);
#endif
                pid = wait(NULL);
#ifdef DEBUG
                printf("Child (%d) finished\n", pid);
#endif
            } 
            else{  /* Child executing the command */

                //this part executes the program 
                if( execvp(args[0], args)) {
                    puts(strerror(errno));
                    exit(127);
                }
            }

        }
    }    
    return 0;
}


