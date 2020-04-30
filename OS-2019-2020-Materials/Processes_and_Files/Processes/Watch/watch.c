#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define BUFFER_SIZE 100

//------------------------------------------------------------------------
// NAME: Georgi Lozanov
// CLASS: XIa
// NUMBER: 11
// PROBLEM: #watch
// FILE NAME: watch.c (unix file name)
// FILE PURPOSE:
// periodic calling of a program using
// the exec family of functions through newly created processes
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// FUNCTION: init_args (име на функцията)
// initialise the dynamic memory to be used by the args container
// PARAMETERS:
// int argc -> argument count for initialising the number of rows (garnered from main)
//------------------------------------------------------------------------

char** init_args(int argc) {
    char** args = malloc(sizeof(char) * argc * BUFFER_SIZE);

    return args;
}

//------------------------------------------------------------------------
// FUNCTION: free_args (име на функцията)
// Free the dynamic memory used by the args container.
// PARAMETERS:
// char** args -> 2D char array containing argument values to be freed from memory alloc'd by init_args
//------------------------------------------------------------------------


void free_args(char** args) {

    free(args);
}

//------------------------------------------------------------------------
// FUNCTION: main (име на функцията)
// main function to execute the program on
// PARAMETERS:
// int argc -> argument count of console arguments
// char** argv -> 2D char array that holds the values of the arguments
//------------------------------------------------------------------------


int main(int argc, char **argv) {
    int status;

    pid_t pid, wait_id;

    char** args = init_args(argc);
    int arg_count = 1;
    char* filename = argv[1];

    for(int i = 0; i < argc; i++) {
        args[i] = argv[arg_count++];
    }

    args[++arg_count] = NULL;
    
    while(1) {
        pid = fork();
        if(pid < 0) {
            /* The fork failed . Report failure . */
            status = -1;
            perror("fork");
        } else if(pid == 0) {
            while(1) {
                sleep(2);
                if(execvp(filename, args) == -1) { //execvp doesn't require absolute path
                    perror(filename);
                }
            }
        } else {
            wait_id = waitpid(pid, &status, 0);
            if(wait_id == -1) {
                perror("Wait");
            }

        }
        
    }

    free_args(args);
}
