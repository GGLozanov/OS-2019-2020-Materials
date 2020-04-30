// pipe -> a way for creating interprocess communication
// pipe -> struct with 2 file descriptors (one for input, one for output)
// redirection of the output of a program to an input of another
// ls > a.txt (output of ls is put inside a.txt)
// ls | wc -> output of ls goes to input of wc (no median files)
// int pipe(int pipefd[2])
// pipefd[0] -> first command
// pipefd[1] -> second command
// returns 0 on success, -1 on error
// process for only reading -> close write descriptor (1) because child reads the
// vice versa for other process (parent process that writes to the other filed)
// handling of partial writes
// pipe only with read() and write()

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 200

int main() {

    int pipefd[2]; // array for the pipe fds
    if(pipe(pipefd) == -1) { // pipe inits by itself pipefd
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    char buffer[BUFFER_SIZE];
    int status;
    if(pid == -1) {
        perror("fork");
        return 1;
    } else if(pid == 0) {

        if(close(pipefd[0]) == -1) { // close write input
            perror("close");
        }

        dup2(pipefd[1], STDOUT_FILENO); 
	// duplicate STDOUT_FILENO fd to write fd of pipefd array as to write the output of execlp() (which is returned to the STDOUT_FILENO fd) to the write file for pipe.

        if(close(pipefd[1]) == -1) { // close read input
            perror("close");
        }

        execlp("ls", "ls", "-l", NULL); // execute the command
    } else {


        if(close(pipefd[1]) == -1) {
            perror("close");
            return 1;
        }

        dup2(pipefd[0], STDIN_FILENO); // duplicate STDIN_FILENO fd to read fd of pipefd array as to read the output of the other execlp() given the recorded output in the STDOUT_FILENO from the ls command in the other process. 
	// The info persists because the pipefd array is separate from ANY of the processes => can use it between them to simulate communication and change the context of the next execlp() call

        // you need to say which file descriptor is for STDIN_FILENO and which is STDOUT_FILENO
        // to simulate pipe()
        // this is done with sys call dup2(int oldfd, int newfd)

        if(close(pipefd[0]) == -1) {
            perror("close");
            return 1;
        }

	execlp("wc", "wc", "-l", NULL);
    }

    return 1;
}
