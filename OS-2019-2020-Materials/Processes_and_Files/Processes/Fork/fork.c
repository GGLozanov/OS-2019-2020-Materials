#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main () {
    int status;
    pid_t pid;
    pid = fork();

    //creates a new process on which the program is executed (which is why when we return 0, we don't stop)
    //fork() creates an EXACT copy of the parent process. 
    //Child processes receive copies of previous variables in the parent process.
    //Ex: int num = 5; 
    //Change the variable somewhere after in the section of the child process 
    //=> changes the variable for the child process
    //but not the parent process

    if(pid < 0) {
        /* The fork failed . Report failure . */
        status = -1;
        printf("Unsuccessful fork ...\n" );
    }
    else if (pid == 0) {
        //in child process
        //sleep(100); -> put time for process in queue for blocked processes
        //fork(); //initialise a new child process in the child process (when we have a child process)
        //execv()

        if(execl("/bin/ls", "/bin/ls", "-l", "-a", NULL) == -1) { 
            //exec() family of functions executes the current process with the given filename
            
            perror("ls");
        } 

        //returns -1 on error. Always finish arguments with NULL
        //return value of exec functions is only for error checking

        return 0; //process finishes (program for the process finishes)
    }
    else {
        //sleep(100);
        //if we use wait(), we put the parent process in the queue of blocked processes
        pid_t wait_status = waitpid(pid, &status, 0); //we can use status with its random address if it's not -1
        //wait() is put in the section of the parent process
        if(wait_status == -1) {
            perror("Wait");
        }
        printf("Hello from unsynchronised parent process (use 'wait' to wait for the child process created by fork!)");
        return 0; //second process (or first) goes to this part of the code and returns 0. (which is why we have 2 outputs)
    }
    return 0;
}
