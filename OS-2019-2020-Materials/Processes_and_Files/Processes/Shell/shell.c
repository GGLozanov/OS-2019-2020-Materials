#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define BUFFER_SIZE 9999

int arguments_count;

//--------------------------------------------
// NAME: Georgi Lozanov
// CLASS: XIa
// NUMBER: 11
// PROBLEM: #3
// FILE NAME: shell.c (unix file name)
// FILE PURPOSE:
// simulation of a simple shell/command interpreter
// using multiprocessing and the exec() family of functions
//---------------------------------------------

//--------------------------------------------
// FUNCTION: count_newspace (име на функцията)
// function that serves to count all the newspaces in a given string 
// (how many rows will be needed for a 2D array that segregates by that margin)
// PARAMETERS:
// const char* buffer -> constant char pointer to represent string to be parsed
//----------------------------------------------

struct Job {
	char* command;
	char** argv;
	int stdin;
	int stdout;
};

//--------------------------------------------
// FUNCTION: init_container (име на функцията)
// function that serves to initialise the 2D array used to store the commands for use by exec() later on
// PARAMETERS:
// int arguments_number -> amount of commands (how many rows) have been given to the shell
//----------------------------------------------

char** init_container(const char* cmdline) {
/*    char** container = malloc(sizeof(char*) * (commands_limit + 1)); // commands_limit + 1 to account for NULL*/

	char** container;
	char delim = ' ';
	char* command;

	container[0] = command = strtok(cmdline, &delim);
	for(arguments_count = 1; command != NULL; arguments_count++) {
		container[arguments_count] = command = strtok(NULL, &delim);
	}

    return container;
}

//--------------------------------------------
// FUNCTION: execute_program (име на функцията)
// function that serves to call execvp on the 2D char array holding the commands
// PARAMETERS:
// char** continer -> 2D char array that holds the commands given by the stdin stream
// int arguments_number -> amount of commands (used when setting the NULL at the end of the 2D char array, which is required by the exec() functions)
//----------------------------------------------

void execute_program(char** commands) {
	if(execvp(*commands, commands) == -1) {
		perror(*commands);
	}
}

//--------------------------------------------
// FUNCTION: parse_cmdline (име на функцията)
// function that serves to return a 2D char array from a single string by separating each individual element by space
// PARAMETERS:
// const char* cmdline -> cmdline given by the user in the shell to be parsed by the function
//----------------------------------------------

char** parse_cmdline(const char* cmdline) {
    return init_container(cmdline);

/*    int word_counter = 0;*/
/*    int letter_counter = 0, len = strlen(cmdline);*/

/*    for(int j = 0; j < len; j++) {*/
/*        if(cmdline[j] == ' ' || cmdline[j] == '\n') {*/
/*	    */
/*		if(cmdline[j + 1] != '\n' && cmdline[j + 1] != '|' && cmdline[j + 1] != '>' && cmdline[j + 1] != '<') {*/
/*			container[word_counter++][letter_counter] = '\0';*/
/*		} else {*/
/*			switch(cmdline[j + 1]) {*/
/*				case '|':*/
/*					// is pipe flag*/
/*					break;*/
/*				case '>':*/
/*					// struct*/
/*					break;*/
/*				case '<':*/
/*					break;*/
/*				default: break;*/
/*			}*/
/*		}*/

/*		letter_counter = 0;*/
/*		continue;*/
/*        }*/
/*        container[word_counter][letter_counter++] = cmdline[j];*/
/*    }*/

/*    return container;*/
}

//--------------------------------------------
// FUNCTION: free_all_memory (име на функцията)
// function that serves to free all of the allocated memory by all of the varaibles using it
// PARAMETERS:
// char** result -> 2D char array that holds the commands given by the stdin stream
// char* buffer -> char array representing the dynamically allocated string given by the user
// int newspaces -> amount of commands (used for individual free()-ing in the destroy_container() function)
//----------------------------------------------

void free_buffer(char* buffer) {
	free(buffer);
}


//--------------------------------------------
// FUNCTION: stdin_read (име на функцията)
// function that serves to simulate the shell functionality, with process forks and shell-like attributes
// PARAMETERS:
// none
//----------------------------------------------

void stdin_read() {
    pid_t pid;
	int status;

    do {
		char* buffer = malloc(sizeof(char) * BUFFER_SIZE);
		printf("$ ");
			
		if(fgets(buffer, BUFFER_SIZE, stdin) == NULL) {
			free(buffer);
			exit(status);
		}
		buffer = strtok(buffer, "\n");

		char** result = parse_cmdline(buffer);
/*		for(int i = 0; i < arguments_count; i++) {*/
/*			printf("str %d: %s\n", i + 1, result[i]);*/
/*		}*/
		
		pid = fork();

		if(pid == -1) {
			status = -1;
			perror("fork");
		} else if(pid == 0) {
			execute_program(result);
			free_buffer(buffer);
			exit(status); //break; here to break the loop for the child process
		} else {
			if(waitpid(pid, &status, 0) == -1) {
				perror("waitpid");
			}
		}
		arguments_count = 1;
		free_buffer(buffer);
	} while(1);

}

//--------------------------------------------
// FUNCTION: main (име на функцията)
// function that serves to call stdin_read()
// PARAMETERS:
// none
//----------------------------------------------

int main() {
	stdin_read();
}
