/*
 *  This is a simple shell program from
 *  rik0.altervista.org/snippetss/csimpleshell.html
 *  It's been modified a bit and comments were added.
 *
 *  It doesn't allow misdirection, e.g., <, >, >>, or |
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#define BUFFER_SIZE 80
#define ARR_SIZE 80

//#define DEBUG 1  /* In case you want debug messages */

void parse_args(char *buffer, char** args, 
                size_t args_size, size_t *nargs)
{
/* 
 * size_t data type is defined in the 1999 ISO C standard (C99).
 * It is used to represent the sizes of objects. size_t is the
 * preferred way to declare arguments or variables that hold the
 * size of an object.
 */
    char *buf_args[args_size]; /* You need C99.  Note that args_size
                                  is normally a constant. */
    char **cp;  /* This is used as a pointer into the string array */
    char *wbuf;  /* String variable that has the command line */
    size_t i, j; 
    
    wbuf=buffer;
    buf_args[0]=buffer; 
    args[0] =buffer;
/*
 * Now 'wbuf' is parsed into the string array 'buf_args'
 *
 * The for-loop uses a string.h function
 *   char *strsep(char **stringp, const char *delim);
 *
 *   Description:  
 *   If *stringp = NULL then it returns NULL and does
 *   nothing else.  Otherwise the function finds the first token in
 *   the string *stringp, where tokens are delimited by symbols
 *   in the string 'delim'.  
 *
 *   In the example below, **stringp is &wbu, and 
 *   the delim = ' ', '\n', and '\t'.  So there are three possible 
 *   delimiters. 
 *
 *   So in the string " Aloha World\n", the spaces and "\n" are
 *   delimiters.  Thus, there are three delimiters.  The tokens
 *   are what's between the delimiters.  So the first token is
 *   "", which is nothing because a space is the first delimiter.
 *   The second token is "Aloha", and the third token is "World".
 *   
 *   The function will scan a character string starting from
 *   *stringp, search for the first delimiter.  It replaces
 *   the delimiter with '\0', and *stringp is updated to point
 *   past the token.  In case no delimiter was found, the
 *   token is taken to be the entire string *stringp, and *stringp
 *   is made NULL.   Strsep returns a pointer to the token. 
 *
 *   Example:  Suppose *stringp -> " Aloha World\n"
 *
 *   The first time strsep is called, the string is "\0Aloha World\n",
 *   and the pointer value returned = 0.  Note the token is nothing.
 *
 *   The second time it is called, the string is "\0Aloha\0World\n",
 *   and the pointer value returned = 1  Note that 'Aloha' is a token.
 *
 *   The third time it is called, the string is '\0Aloha\0World\0', 
 *   and the pointer value returned is 7.  Note that 'World' is a token.
 *
 *   The fourth time it is called, it returns NULL.
 *
 *   The for-loop, goes through buffer starting at the beginning.
 *   wbuf is updated to point to the next token, and cp is
 *   updated to point to the current token, which terminated by '\0'.
 *   Note that pointers to tokens are stored in array buf_args through cp.
 *   The loop stops if there are no more tokens or exceeded the
 *   array buf_args.
 */   
    /* cp is a pointer to buff_args */ 
    for(cp=buf_args; (*cp=strsep(&wbuf, " \n\t<>|")) != NULL ;){
        if ((*cp != '\0') && (++cp >= &buf_args[args_size]))
            break; 
    }

/* 
 * Copy 'buf_args' into 'args'
 */    
    for (j=i=0; buf_args[i]!=NULL; i++){ 
        if(strlen(buf_args[i])>0)  /* Store only non-empty tokens */
            args[j++]=buf_args[i];
    }
    
    *nargs=j;
    args[j]=NULL;
}

struct command {
   char **argv;
};

struct buffers
{
char temp[BUFFER_SIZE];
char  *args[ARR_SIZE];
};

//struct pipes
//{
//int in[2];
//int out[2];
//};

int spawn_process(int in, int out, struct command *cmd)
{
    pid_t pid;

    if((pid = fork()) == 0)
    {
        if(in != 0)
        {
            dup2(in, 0);
            close(in);
        }
        if(out != 1)
        {
            dup2(out, 1);
            close(out);
        }
        return execvp(cmd->argv[0], (char * const *)cmd->argv);
    }

    return pid;
}

int fork_pipes(int n, struct command *cmd) 
{
    int i;
    pid_t pid;
    int in, fd[2];

    /* The first process should get its input from the original file descriptor 0 */
    in = 0;

    /* Note the loop bound, we spawn here all, but the last stage of the pipeline */
    for(i = 0; i < n -1; ++i)
    {
        pipe(fd);

        /* fd[1] is the write end of the pipe, we carry "in" from the previous iteration */
        spawn_process(in, fd[1], cmd + i);

        /* No need for the write end of the pipe, the child will write here */
        close(fd[1]);

        /* Keep the read end of the pipe, the next child will read from there */
        in = fd[0];
    }
    /* Last stage of the pipeline - set stdin be the read end of the previous pipe and output to the original file descriptor 1 */
    if(in != 0)
    {
        dup2(in,0);
    }
    /* Execute the last stage with the current process */
    return execvp(cmd[i].argv[0], (char * const *)cmd[i].argv);
}


int main(int argc, char *argv[], char *envp[]){
    char buffer[BUFFER_SIZE];
    char *args[ARR_SIZE];

    int *ret_status;
    size_t nargs;
    pid_t pid;
    
    while(1){
        printf("ee468>> "); /* Prompt */
        fgets(buffer, BUFFER_SIZE, stdin); /* Read in command line */

	//Count to see how much "|" symbols are in string
	int i, count;
	int pipe_location[9];
	int j = 0;
	
	struct buffers myBuffers[10];
	for(i = 0, count = 0; buffer[i]; i++)
	{
		if(buffer[i] == '|')
		{
			pipe_location[count] = i;
			count+=1;
		}
	}

	//Parse each Buffer
	for(i = 0; i<=count; i++)
	{
		int k = 0;
		if(i == count)//Last arguements
		{
			for(j; j<BUFFER_SIZE; j++)
			{
				myBuffers[i].temp[k] = buffer[j];
				k++;
			}
		parse_args(myBuffers[i].temp, myBuffers[i].args, ARR_SIZE, &nargs);
		}
		else
		{
		for (j; j<pipe_location[i]; j++)
		{
			myBuffers[i].temp[k] = buffer[j];
			k++;
		}
		j = pipe_location[i]+1;
		parse_args(myBuffers[i].temp, myBuffers[i].args, ARR_SIZE, &nargs);
		}
	}
#ifdef DEBUG
for(i = 0; myBuffers[1].args[i]!=NULL; i++)
{
	printf("The subCommand is : %s\n", myBuffers[1].args[i]);
}
#endif
		
//	parse_args(buffer, args, ARR_SIZE, &nargs); 
        if (nargs==0) continue; /* Nothing entered so prompt again */
//        if (!strcmp(args[0], "exit" )) exit(0);     

#ifdef DEBUG
//for(i = 0; args[i]!=NULL; i++){
//	printf("The command is : %s\n", args[i]);
//}
printf("Number of pipes is: %d\n", count);

#endif
	
	//PIPES
	if(count > 0)
	{
	pid = fork();
//	struct pipes myPipes[9];
//	printf("There are pipes\n");
 	if(pid == 0)
	{
		if(count == 1) //Only 1 pipe
		{
			struct command cmd[] = {myBuffers[0].args, myBuffers[1].args};
			fork_pipes(2, cmd);
		}
		else if(count == 2)
		{
			struct command cmd[] = {{myBuffers[0].args}, {myBuffers[1].args}, {myBuffers[2].args}};
			fork_pipes(3, cmd);
		}
		else if(count == 3)
		{
			struct command cmd[] = {{myBuffers[0].args}, {myBuffers[1].args}, {myBuffers[2].args}, {myBuffers[3].args}};
			fork_pipes(4, cmd);
		}
		else if(count == 4)
		{
			struct command cmd[] = {{myBuffers[0].args}, {myBuffers[1].args}, {myBuffers[2].args}, {myBuffers[3].args}, {myBuffers[4].args}};
			fork_pipes(5, cmd);
		}
		else if(count == 5)
		{
			struct command cmd[] = {{myBuffers[0].args}, {myBuffers[1].args}, {myBuffers[2].args}, {myBuffers[3].args}, {myBuffers[4].args}, {myBuffers[5].args}};
			fork_pipes(6, cmd);
		}
		else if(count == 6)
		{
			struct command cmd[] = {{myBuffers[0].args}, {myBuffers[1].args}, {myBuffers[2].args}, {myBuffers[3].args}, {myBuffers[4].args}, {myBuffers[5].args}, {myBuffers[6].args}};
			fork_pipes(7, cmd);
		}
		else if(count == 7)
		{
			struct command cmd[] = {{myBuffers[0].args}, {myBuffers[1].args}, {myBuffers[2].args}, {myBuffers[3].args}, {myBuffers[4].args}, {myBuffers[5].args}, {myBuffers[6].args}, {myBuffers[7].args}};
			fork_pipes(8, cmd);
		}
		else if(count == 8)
		{
			struct command cmd[] = {{myBuffers[0].args}, {myBuffers[1].args}, {myBuffers[2].args}, {myBuffers[3].args}, {myBuffers[4].args}, {myBuffers[5].args}, {myBuffers[6].args}, {myBuffers[7].args}, {myBuffers[8].args}};
			fork_pipes(9, cmd);
		}
		else if(count == 9)
		{
			struct command cmd[] = {{myBuffers[0].args}, {myBuffers[1].args}, {myBuffers[2].args}, {myBuffers[3].args}, {myBuffers[4].args}, {myBuffers[5].args}, {myBuffers[6].args}, {myBuffers[7].args}, {myBuffers[8].args}, {myBuffers[9].args}};
			fork_pipes(10, cmd);
		}
	}
	}		//NO PIPES
	else    
	{
	parse_args(buffer, args, ARR_SIZE, &nargs);
	if (!strcmp(args[0], "exit")) exit(0);	
	
        pid = fork();

        if (pid){  /* The parent, for debugging purposes */
#ifdef DEBUG
            printf("Waiting for child (%d)\n", pid);
#endif
            pid = wait(ret_status);
#ifdef DEBUG
            printf("Child (%d) finished\n", pid);
#endif
        } 

        else{  /* The child executing the command */
            if( execvp(args[0], args)) {
                puts(strerror(errno));
                exit(127);
        }

        }}
	if(pid)
	{
		wait(ret_status);
	}
    }    
    return 0;
}
