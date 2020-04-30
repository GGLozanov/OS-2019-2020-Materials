#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

//------------------------------------------------------------------------
// NAME: Georgi Lozanov
// CLASS: XIa
// NUMBER: 11
// PROBLEM: #1
// FILE NAME: lseek_reverse.c (unix file name)
// FILE PURPOSE:
// Simulation of encrypting a message using the block type structure and the lseek sys call.
// 
//------------------------------------------------------------------------

#define BUFFER_SIZE 1
#define FILE_OFFSET 50
#define MESSAGE_LENGTH 129

//SEEK_SET -> exactly offset bytes
//SEEK_CUR -> current location + offset
//off_t -> signed long int


//------------------------------------------------------------------------
// STRUCTURE: block (име на функцията)
// PARAMETERS:
// char data -> holds a byte of data
// unsigned char nextElementAddress -> an unsigned char that holds the address for the next element. Used for lseek sys call.
//------------------------------------------------------------------------

typedef struct {
	char data;
	unsigned char nextElementAddress;
} block;

block blocks[128];

//------------------------------------------------------------------------
// FUNCTION: scan (име на функцията)
// Scans the file using the block structure and outputs the result with write()
// PARAMETERS:
// fileInd for read and write sys calls. Tells you whether the file is open.
//------------------------------------------------------------------------

void scan(int fileInd) {
    lseek(fileInd, 0, SEEK_SET); //reset the file offset with lseek() again as to start from the beginning
	int i = 0;

	while(1) {
		read(fileInd, &blocks[i].data, BUFFER_SIZE);
		//read(fileInd, &blocks[i].nextElementAddress, BUFFER_SIZE);
		write(STDOUT_FILENO, &blocks[i].data, BUFFER_SIZE);
		lseek(fileInd, blocks[i].nextElementAddress, SEEK_SET); //change the offset to the current one + the next address
		//printf("\nAddress 2: %d; data 2 -> %c", i, blocks[i].data);
		i = blocks[i].nextElementAddress;

		if(!i) break;

	}
	write(STDOUT_FILENO, "\n", BUFFER_SIZE);
}

//------------------------------------------------------------------------
// FUNCTION: encrypt (име на функцията)
// Encrypts the given message using the write() sys call and mixes up the elements using lseek()
// PARAMETERS:
// message[MESSAGE_LENGTH] -> message for encryption
// fileInd -> for read and write sys calls. Tells you whether the file is successfully opened.
//------------------------------------------------------------------------

void encrypt(char* message, int fileInd) {
	int rand_position = 0, next_element_position = 0;

	for(int i = 0; i < strlen(message) - 1; i++) {
		rand_position = i == 0 ? 0 : blocks[rand_position].nextElementAddress;
		//printf("Block position: %d; data -> %c\n", rand_position, message[i]);
        blocks[rand_position].data = message[i];
		write(fileInd, &blocks[rand_position].data, BUFFER_SIZE);

        blocks[rand_position].nextElementAddress = i == strlen(message) - 2 ? 0 : rand() % MESSAGE_LENGTH;
		if(!blocks[rand_position].nextElementAddress) break;

        write(fileInd, &blocks[rand_position].nextElementAddress, BUFFER_SIZE);
        lseek(fileInd, blocks[rand_position].nextElementAddress, SEEK_SET); //mix up the file with the current offset + bytes equal to the address of the next element
		//printf("\nAddress: %d", blocks[rand_position].nextElementAddress);
    }
}

//------------------------------------------------------------------------
// FUNCTION: main (име на функцията)
// Main function that opens and closes the file while executing the scan() function in-between
// PARAMETERS:
// int argc -> console argument amount
// char **argc -> console argument value represented as a 2D char array.
//------------------------------------------------------------------------


int main(int argc, char** argv) {
	srand(time(NULL));
    char message[MESSAGE_LENGTH];
    ssize_t read_value = read(STDIN_FILENO, message, MESSAGE_LENGTH);
    message[read_value] = '\0'; //since STDIN_FILENO doesn't pass in the terminating null character, you have to pass it (learned that the hard way)
	int fileInd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC); //truncate the entire file if it exists (to avoid conflicts with other, previous inputs)

	if(fileInd == -1) {
		perror("Open file");
		return 1;
	}

	encrypt(message, fileInd);

	int closeInd = close(fileInd);
	if(closeInd == -1) {
		perror("Close file");
		return 1;
	}
}
