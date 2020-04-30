#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 1

//------------------------------------------------------------------------
// NAME: Georgi Lozanov
// CLASS: XIa
// NUMBER: 11
// PROBLEM: #1
// FILE NAME: tail.c (unix file name)
// FILE PURPOSE:
// Simulating the linux command tail() using lseek() with SEEK_END whence, along with
// other extra features, such as error handling and writing from stdin stream.
//------------------------------------------------------------------------

//------------------------------------------------------------------------
// FUNCTION: write_input_error (име на функцията)
// using the write syscall and the STDERR_FILENO macro, we write the appropriate error message
// to the stderr stream, along with the origin of its error, which is woven from perror("")
// used when an error occurs in writing.
// PARAMETERS:
// char* filename -> char pointer to a variable filename. Used to represent the string for the filename. 
//------------------------------------------------------------------------

void write_input_error(char* filename) {
    write(STDERR_FILENO, "tail: error writing '", 21);
    write(STDERR_FILENO, filename, strlen(filename));
    write(STDERR_FILENO, "': ", 3);
    perror("");
}

//------------------------------------------------------------------------
// FUNCTION: write_output_error (име на функцията)
// using the write syscall and the STDERR_FILENO macro, we write the appropriate error message
// to the stderr stream, along with the origin of its error, which is woven from perror("")
// used when there is an error in reading or the given filename is a directory.
// PARAMETERS:
// char* filename -> char pointer to a variable filename. Used to represent the string for the filename. 
//------------------------------------------------------------------------

void write_output_error(char* filename) {
    write(STDERR_FILENO, "tail: error reading '", 21);
    write(STDERR_FILENO, filename, strlen(filename));
    write(STDERR_FILENO, "': ", 3);
    perror("");
}

//------------------------------------------------------------------------
// FUNCTION: write_no_file_error (име на функцията)
// using the write syscall and the STDERR_FILENO macro, we write the appropriate error message
// to the stderr stream, along with the origin of its error, which is woven from perror("").
// used when there is no file or directory with the given filename.
// PARAMETERS:
// char* filename -> char pointer to a variable filename. Used to represent the string for the filename. 
//------------------------------------------------------------------------

void write_no_file_error(char* filename) {
    write(STDERR_FILENO, "tail: cannot open '", 19);
    write(STDERR_FILENO, filename, strlen(filename));
    write(STDERR_FILENO, "' for reading: ", 15);
    perror("");
}

//------------------------------------------------------------------------
// FUNCTION: write_no_space_error (име на функцията)
// using the write syscall and the STDERR_FILENO macro, we write the appropriate error message
// to the stderr stream, along with the origin of its error, which is woven from perror("")
// used when we write to a full directory with no space.
// PARAMETERS:
// char* filename -> char pointer to a variable filename. Used to represent the string for the filename. 
//------------------------------------------------------------------------

void write_no_space_error() {
    write(STDERR_FILENO, "tail: error writing 'standard output': ", 39);
    perror("");
}

//------------------------------------------------------------------------
// FUNCTION: write_header (име на функцията)
// function to write the header format when called on a given file to the stdout stream using
// write syscall and STDOUT_FILENO macro file descriptor
// PARAMETERS:
// char* filename -> char pointer to a variable filename. Used to represent the string for the filename. 
//------------------------------------------------------------------------

void write_header(char* filename) {
    write(STDOUT_FILENO, "==> ", 4); //header writing
    write(STDOUT_FILENO, filename, strlen(filename));
    write(STDOUT_FILENO, " <==", 4);
    write(STDOUT_FILENO, "\n", 1);
}

//------------------------------------------------------------------------
// FUNCTION: tail (име на функцията)
// the core of the program; utilising lseek() and a constantly decrementing traceback variable, we
// go back from the end of the file, attest for each symbol and break if it's smaller/equal to than ten lines or we've read up to ten lines
// after which we go from our current seek to the end of the file, reading character by character.
// PARAMETERS:
// char* filename -> char pointer to a variable filename. Used to represent the string for the filename.
// int fileInd -> file descriptor variable used for the syscalls in the function. Attained from main()
//------------------------------------------------------------------------

void tail(char* filename, int fileInd) {
    int traceback_size = -1; //size for going back to the last 10th line
    int num_lines = 10; //number of lines we count to
    char buffer[BUFFER_SIZE]; //temp buffer which keeps track of characters
    ssize_t read_buffer, write_buffer; //buffers for how many bytes are read

    while(num_lines && read_buffer) { //loop until we either have 0 lines or we've read the entire file
        lseek(fileInd, --traceback_size, SEEK_END); 
        //seek to SEEK_END (end), and go back traceback_size - 1
        //-1 byte -> last character; -2 -> penultimate and so on.
        read_buffer = read(fileInd, buffer, BUFFER_SIZE); //read said character we've traced back to until we read 0 (partial read handling)

        if(read_buffer == -1 && (errno == EIO || errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) { //read error check
            write_output_error(filename);
            return;
        }
        if(*buffer == '\n') num_lines--; //newline check that decrements newline upon finding one
    }

    if(!read_buffer) { //seek back to the beginning if the file is less than/equal to 10 lines (read it again)
        lseek(fileInd, 0, SEEK_SET);
    }
    
    while(read_buffer = read(fileInd, buffer, BUFFER_SIZE)) { //reading the file from where we are
        if(read_buffer == -1 && (errno == EIO || errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) { //error check
            write_output_error(filename);
            return;
        }

	    do {
            write_buffer = write(STDOUT_FILENO, buffer, BUFFER_SIZE); //writing to standard output
        } while(!write_buffer); //handling the partial writing by writing while our write value is 0 (this condition is overloaded by the read() until != 0 loop)

        if(write_buffer == -1 && (errno == EIO || errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) { //error check
            write_input_error(filename);
            return;
        }

        if(errno == ENOSPC) { //no space check while writing
            write_no_space_error();
            return;
        }
    }
}

//------------------------------------------------------------------------
// FUNCTION: stdio_read (име на функцията)
// helper function which we use to call tail() after we receive input from the stdin stream
// using a static file with permissions for all users, in which we write the input from stdin
// after which we call tail() with the file descriptor for the "stdin.txt" temp file.
// PARAMETERS:
// no parameters. always reads from STDIN_FILENO.
//------------------------------------------------------------------------

void stdio_read() {
    char buffer[BUFFER_SIZE]; //temporary buffer of size 1 byte
    char* filename = "stdin.txt";
    ssize_t read_buffer, write_buffer; //buffers for storage of size read
    int fileInd = open(filename, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO); 
    //opening the file "stdin.txt" with flags for creating, reading and writing, and truncating (setting its size to 0) if it already exists.
    //The rest of the flags are permission flags separated by bitwise ORs (permission for owner (User), groups (Group) and all others (Others))

    while(read_buffer = read(STDIN_FILENO, buffer, BUFFER_SIZE)) { //reading until the user doesn't press EOF (CTRL+D), which read() sees as '0'
        if(read_buffer == -1 && (errno == EIO || errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) {
            write_output_error(filename);
            return;
        }
        
        do {
            write_buffer = write(fileInd, buffer, BUFFER_SIZE); //writing to standard output
        } while(!write_buffer); //writing the characters read to the static file while we receive a 0 (handles the partial writing because it's broken by the read() loop from EOF)

        if(write_buffer == -1 && (errno == EIO || errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)) { //error check
            write_input_error(filename);
            return;
        }
    }

    tail(filename, fileInd); //calling tail() on the file
    int close_indicator = close(fileInd); //closing the file

    if(close_indicator == -1 && errno == EIO) { //error check
        write_output_error(filename);
        return;
    }
}

//------------------------------------------------------------------------
// FUNCTION: main (име на функцията)
// the main function in which we call all other functions, implement the header format
// and open each individual given file
// PARAMETERS:
// int argc -> console argument count
// char** argv -> char pointer to string values for console arguments.
//------------------------------------------------------------------------

int main(int argc, char **argv) {
	if(argc < 2) { //if we have no arguments (less than two.) The output command counts as an argument as well.
        stdio_read(); //read from stdin function
	}

	for(int i = 1; i < argc; i++) { //starting the main loop for parsing through the files from 1 (for reasons stated above)
		char* filename = argv[i]; //put the value of the current argument into the char pointer -> filename

        if(*filename == '-') { //dereference value at pointer with * (first element of arr) and check whether it is equal to '-' for stdin stream
            stdio_read(); //read from stdin function
            continue; //continue to next file
        }

	    int fileInd = open(filename, O_RDWR); 
        //opening for both read and write due to EISDIR only being triggered
        //upon attempts to write to a directory (or opening it for writing).
        int close_indicator;

        if(fileInd == -1) { //error check
            switch(errno) { //switch statement for two cases of errno value (errno -> global error variable)
                case ENOENT:
                    write_no_file_error(filename);
                    break;
                case EISDIR:
                    write_output_error(filename);
                    break;
                default: break;
            }
            close_indicator = close(fileInd); //same principle for close()
            if(close_indicator == -1 && errno == EIO) { //different output ("Bad file descriptor") due to indicator being for a different sys call and Error_number
                write_output_error(filename);
		    }
            continue;
		}

		if(argc >= 3) { //only write header if there is more than one file
            write_header(filename);
		}

		tail(filename, fileInd); //call tail() on file

		close_indicator = close(fileInd); //same principle for close()
		//returns 0 if operation is successful

		if(close_indicator == -1 && errno == EIO) { //different output ("Bad file descriptor") due to indicator being for a different sys call and Error_number
            write_output_error(filename); //error check
            continue;
		}
		if(argc >= 3 && i != argc - 1) write(STDOUT_FILENO, "\n", 1); //final newline writing (except on final file or 1 file)
	}
    //exit(errno);
}
