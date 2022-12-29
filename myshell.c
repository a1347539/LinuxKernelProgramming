/*
    COMP3511 Fall 2022 
    PA1: Simplified Linux Shell (MyShell)

    Your name: Li Chun Tak
    Your ITSC email: ctliaj@connect.ust.hk 

    Declaration:

    I declare that I am not involved in plagiarism
    I understand that both parties (i.e., students providing the codes and students copying the codes) will receive 0 marks. 

*/

// Note: Necessary header files are included
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/stat.h> // For constants that are required in open/read/write/close syscalls
#include <sys/wait.h> // For wait() - suppress warning messages
#include <fcntl.h> // For open/read/write/close syscalls

// Assume that each command line has at most 256 characters (including NULL)
#define MAX_CMDLINE_LEN 256

// Assume that we have at most 8 arguments
#define MAX_ARGUMENTS 8

// Assume that we only need to support 2 types of space characters: 
// " " (space) and "\t" (tab)
#define SPACE_CHARS " \t"

// The pipe character
#define PIPE_CHAR "|"

// Assume that we only have at most 8 pipe segements, 
// and each segment has at most 256 characters
#define MAX_PIPE_SEGMENTS 8

// Assume that we have at most 8 arguments for each segment
//
// We also need to add an extra NULL item to be used in execvp
//
// Thus: 8 + 1 = 9
//
// Example: 
//   echo a1 a2 a3 a4 a5 a6 a7 
//
// execvp system call needs to store an extra NULL to represent the end of the parameter list
//
//   char *arguments[MAX_ARGUMENTS_PER_SEGMENT]; 
//
//   strings stored in the array: echo a1 a2 a3 a4 a5 a6 a7 NULL
//
#define MAX_ARGUMENTS_PER_SEGMENT 9

// Define the  Standard file descriptors here
#define STDIN_FILENO    0       // Standard input
#define STDOUT_FILENO   1       // Standard output 


 
// This function will be invoked by main()
// TODO: Implement the multi-level pipes below
void process_cmd(char *cmdline);

// read_tokens function is given
// This function helps you parse the command line
// Note: Before calling execvp, please remember to add NULL as the last item 
void read_tokens(char **argv, char *line, int *numTokens, char *token);

// Here is an example code that illustrates how to use the read_tokens function

/*
int main() {
    char *pipe_segments[MAX_PIPE_SEGMENTS]; // character array buffer to store the pipe segements
    int num_pipe_segments; // an output integer to store the number of pipe segment parsed by this function
    char cmdline[MAX_CMDLINE_LEN]; // the input argument of the process_cmd function
    int i, j;
    char *arguments[MAX_ARGUMENTS_PER_SEGMENT] = {NULL}; 
    int num_arguments;
    strcpy(cmdline, "ls | sort -r | sort | sort -r | sort | sort -r | sort | sort -r");
    read_tokens(pipe_segments, cmdline, &num_pipe_segments, PIPE_CHAR);
    for (i=0; i< num_pipe_segments; i++) {
        printf("%d : %s\n", i, pipe_segments[i] );    
        read_tokens(arguments, pipe_segments[i], &num_arguments, SPACE_CHARS);
        for (j=0; j<num_arguments; j++) {
            printf("\t%d : %s\n", j, arguments[j]);
        }
    }
    return 0;
}
*/

/* The main function implementation */

int main()
{
    char cmdline[MAX_CMDLINE_LEN];
    fgets(cmdline, MAX_CMDLINE_LEN, stdin);

    size_t ln = strlen(cmdline);
    if (ln > 0 && cmdline[ln-1] == '\n') 
        cmdline[--ln] = '\0';

    process_cmd(cmdline);
    return 0;
}


// TODO: implementation of process_cmd
void process_cmd(char *cmdline)
{
    // file operation
    int fd;
    // pipe operation
    int pfds[2];
    int fdi = 0;

    int directIndex = MAX_ARGUMENTS_PER_SEGMENT;

    char *pipe_segments[MAX_PIPE_SEGMENTS]; // character array buffer to store the pipe segements
    int num_pipe_segments; // an output integer to store the number of pipe segment parsed by this function
    int i, j;
    char *arguments[MAX_ARGUMENTS_PER_SEGMENT] = {NULL}; 
    int num_arguments;
    read_tokens(pipe_segments, cmdline, &num_pipe_segments, PIPE_CHAR);
    for (i=0; i< num_pipe_segments; i++) {
        // pipe operation
        pipe(pfds);
        
        read_tokens(arguments, pipe_segments[i], &num_arguments, SPACE_CHARS);
        
        // file processing starts
        for (int j = 0; j < num_arguments; j++) {
            if (strcmp(arguments[j], "<") == 0) {
            if (j < directIndex) directIndex = j;
                fd = open(arguments[j+1], O_RDONLY, S_IRUSR | S_IWUSR);
            close(0);
            dup(fd);
            close(fd);
            } else if (strcmp(arguments[j], ">") == 0) {
            if (j < directIndex) directIndex = j;
                fd = open(arguments[j+1], O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR);
            close(1);
            dup(fd);
            close(fd);
            }
        }	
        // file processing ends
        
        if (fork()) {
            // parent
            wait(NULL);
            close(pfds[1]);
            fdi = pfds[0];
            
            for (int i = 0; i < MAX_ARGUMENTS_PER_SEGMENT; ++i) {
                arguments[i] = NULL;
            }
        }
        else {
            // child

            // for file processing
            if (directIndex < MAX_ARGUMENTS_PER_SEGMENT) {
            num_arguments = directIndex;
            for (int i = directIndex; i < directIndex + 4; ++i) {
                arguments[i] = NULL;
            }
            }

            // print outs
                // for (j=0; j<num_arguments; j++) {
            //     printf("param %d: %s ", j ,arguments[j]);
                // }
            // printf("\n");

            // pipe operation
            // printf("%d", fdi);
            if (fdi != 0) {
                close(0);
            dup(fdi);
            }

            if (i < num_pipe_segments-1) {
            close(pfds[0]);
                close(1);
            dup(pfds[1]);
            
            }	

            execvp(arguments[0], arguments);
        }
    }
    // You can try to write: printf("%s\n", cmdline); to check the content of cmdline
}

// Implementation of read_tokens function
void read_tokens(char **argv, char *line, int *numTokens, char *delimiter)
{
    int argc = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    *numTokens = argc;
}
