#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <errno.h>
#include <sys/time.h>

#define A_FLAG_MASK 0b0001 // bit masks used for identifying the flags given
#define L_FLAG_MASK 0b0010
#define R_FLAG_MASK 0b0100
#define NO_ARG_MASK 0b1000 // bit mask for when no arguments are given to the program
#define ALL_FLAG_MASK (A_FLAG_MASK | L_FLAG_MASK | R_FLAG_MASK) // 0b111
#define OPT_LIST "ARl" // options list
#define DEFAULT_DIR "."

static short command_flags = 0b0000;

void write_not_exist_file_error(char* filename) {
    write(STDERR_FILENO, "ls: cannot access ", 18);
    write(STDERR_FILENO, filename, strlen(filename));
    write(STDERR_FILENO, ": ", 2);
    perror(": ");
}

void write_no_access_directory_error(char* dirname) {
    write(STDERR_FILENO, "ls: cannot open directory ", 26);
    write(STDERR_FILENO, dirname, strlen(dirname));
    perror(": ");
}

char get_file_permissions(struct stat st) {
	st.st_mode &= ~S_IFMT; // AND out the unnecessary bits and get those needed for file permission (by NOT-in the macro)
	// lots of ifs...
}

char get_file_prefix(struct stat st) { // sadly, you can't access the mode_t struct :(
	if(S_ISREG(st.st_mode)) {
		return '-';
	}
	if(S_ISDIR(st.st_mode)) {
		return 'd';
	}
	if(S_ISSOCK(st.st_mode)) {
		return 's';
	}
	if(S_ISLNK(st.st_mode)) {
		return 'l';
	}
	if(S_ISBLK(st.st_mode)) {
		return 'b';
	}
	if(S_ISCHR(st.st_mode)) {
		return 'c';
	}
	if(S_ISFIFO(st.st_mode)) {
		return 'p';
	}
}

/* void ls_file_a(struct stat* st) {
	// magic with hidden files
	// ls_file_default()
} */

void ls_file_r(struct stat* st) {
	// leave the recursive calls for the directory and have it handle; just read file normally
	// probably don't need this function
}

void ls_file_l(struct stat* st) {
	// check forbidden files
	// implement logic here w/ everything
	// get permissions, deep links, users/groups, time w/ localtime(), file/dir name
	// ternary for permissions
}

void ls_file_default(struct stat* st) {

}

void ls_file(struct stat* st) {
	if(!(command_flags & ALL_FLAG_MASK)) {
		ls_file_default(st);
	} else {
		// if(command_flags & A_FLAG_MASK) ls_file_a(st);
		if(command_flags & L_FLAG_MASK) ls_file_r(st);			
		if(command_flags & R_FLAG_MASK) ls_file_l(st);
	}
}

void ls_dir_a(struct dirent* dir_entry) {
	// call ls_dir_default() w/ mode for hidden files set to true
}

void ls_dir_r(struct dirent* dir_entry) {
	// recursive call to ls()
}

void ls_dir_l(struct dirent* dir_entry) { // pass in copy of direntry, not ptr? Teacher told us to copy our direntries
	// check NO_ARG_MASK here
	// ls_file_l()
}

void ls_dir_default(struct dirent* dir_entry) { // handles both -A and default
	if(*dir_entry->d_name != '.' || (*dir_entry->d_name == '.' && command_flags & A_FLAG_MASK)) {
		struct stat dir_stat;
		stat(dir_entry->d_name, &dir_stat);
		printf("%c %s\n", get_file_prefix(dir_stat), dir_entry->d_name);
	}
}


// also calls ls_file
void ls_dir(struct stat* st, char* path) {
	DIR* dir;
	
	if((dir = opendir(path)) == NULL) {
		write_no_access_directory_error(path);
		return;
	} // error check here
	
	struct dirent* dir_entry;
		
	if(!(command_flags & L_FLAG_MASK) && !(command_flags & NO_ARG_MASK)) printf("d %s:\n", path); 
	// if not called with -l AND there is at least one argument
	
	// errno = 0;
	while((dir_entry = readdir(dir)) != NULL) {
	    // optimise bool condition
		if(!(command_flags & (L_FLAG_MASK | R_FLAG_MASK)) || command_flags & A_FLAG_MASK) {
			ls_dir_default(dir_entry); 
			// ls_dir_default() handles both -A and default 
			// (which is why check is like that and doesn't use ALL_FLAG_MASK)
		} else {
			if(command_flags & L_FLAG_MASK) ls_dir_r(dir_entry);			
			if(command_flags & R_FLAG_MASK) ls_dir_l(dir_entry);
		}
	}
	/* if(errno != 0) {
		// error has occurred with reading the directory
	} */
	closedir(dir);
}

void ls(char* path) {
	// determines apt functions by calling stat() and ls_type_arg if needed
	// 1 - for directories; 0 - files
	struct stat st;
	if(stat(path, &st) == -1) {
		write_not_exist_file_error(path);
		return;
	} // error check here

	// might need to move AND to prefix function

	
	if(S_ISDIR(st.st_mode)) {
		ls_dir(&st, path);
	} else ls_file(&st);

}

int main(int argc, char** argv) {
	int opt;
	int idx;
	// char** opt_args;
	
	while((opt = getopt(argc, argv, OPT_LIST)) != -1) {
		switch(opt) {
			case 'A':
				printf("Received -A as arg!\n");
				printf("arg: %s\n", optarg);
				command_flags |= A_FLAG_MASK; // add the -A flag mask to the command_flag var
				break;
			case 'l':
				printf("Received -l as arg!\n");
				printf("arg: %s\n", optarg);
				command_flags |= L_FLAG_MASK; // add the -l flag mask to the command_flag var
				break;
			case 'R':
				printf("Received -R as arg!\n");
				printf("arg: %s\n", optarg);
				command_flags |= R_FLAG_MASK; // add the -R flag mask to the command_flag var
				break;
			default: break;
		}
	}
	
	// set the opt index back to the start
	// cond -> *argv[optind] != '-' ?
	if(optind == argc) {
		command_flags |= NO_ARG_MASK;
		ls(DEFAULT_DIR);
		exit(0);
	}
	
	for(idx = optind; idx < argc; idx++) {
		// ls(argv[optind]);
		printf("Arg: %s\n", argv[idx]);
	}
	// run stat() and check if dir => do stuff depending on type
}
