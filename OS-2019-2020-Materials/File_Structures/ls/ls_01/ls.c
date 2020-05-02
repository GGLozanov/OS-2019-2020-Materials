//--------------------------------------------
// NAME: Georgi Lozanov
// CLASS: XIa
// NUMBER: 11
// PROBLEM: #4
// FILE NAME: ls.c (unix file name)
// FILE PURPOSE:
// A simulation of the common UNIX command ls (list segments)
// used to list the contents of a directory and/or file
// options include: -R, -A, -l
//---------------------------------------------

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <dirent.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <locale.h>
#include <langinfo.h>

#define A_FLAG_MASK 0b0001 // bit masks used for identifying the flags given
#define L_FLAG_MASK 0b0010
#define R_FLAG_MASK 0b0100
#define NO_ARG_MASK 0b1000 // bit mask for when no arguments are given to the program
#define ALL_FLAG_MASK (A_FLAG_MASK | L_FLAG_MASK | R_FLAG_MASK) // 0b0111
#define OPT_LIST "ARl" // options list
#define DEFAULT_DIR "."
#define PERMISSIONS_SIZE 11

// NOTE: ALL_FLAG_MASK and NO_ARG_MASK are two different bit masks. The former describes when no optional arguments are received from getopt(), while the latter designates no arguments (files, dirs) given to the program

static short command_flags = 0b0000; // command flag bits used to indicate whether there are any flags, and if so - which are chosen
int idx; // optind idx (follows through the program up to argc and used to check spacing)
char source_dir[PATH_MAX]; // source directory wherein ls is first called

//--------------------------------------------
// FUNCTION: write_not_exist_file_error
// outputs the error message when a certain file/directory cannot be found
// PARAMETERS:
// char* filename - the name of the file/directory which cannot be found
//----------------------------------------------
void write_not_exist_file_error(char* filename) {
    write(STDERR_FILENO, "ls: cannot access ", 18);
    write(STDERR_FILENO, filename, strlen(filename));
    write(STDERR_FILENO, ": ", 2);
    perror("");
}

//--------------------------------------------
// FUNCTION: write_no_access_directory_error
// outputs the error message when a certain file/directory cannot be accessed due to permission restrictions
// PARAMETERS:
// char* filename - the name of the file/directory which cannot be accessed
//----------------------------------------------
void write_no_access_directory_error(char* dirname) {
    write(STDERR_FILENO, "ls: cannot open directory ", 26);
    write(STDERR_FILENO, dirname, strlen(dirname));
    write(STDERR_FILENO, ": ", 2);
    perror("");
}

//--------------------------------------------
// FUNCTION: is_r_flag
// returns whether the -R flag has been chosen in the invocation of the program through bitwise AND-ing the mask for said flag
// PARAMETERS:
// none/null/nil, whatever your preference
//----------------------------------------------
int is_r_flag() {
	return command_flags & R_FLAG_MASK;
}

//--------------------------------------------
// FUNCTION: is_l_flag
// returns whether the -l flag has been chosen in the invocation of the program through bitwise AND-ing the mask for said flag
// PARAMETERS:
// none/null/nil, whatever your preference
//----------------------------------------------
int is_l_flag() {
	return command_flags & L_FLAG_MASK;
}

//--------------------------------------------
// FUNCTION: is_a_flag
// returns whether the -A flag has been chosen in the invocation of the program through bitwise AND-ing the mask for said flag
// PARAMETERS:
// none/null/nil, whatever your preference
//----------------------------------------------
int is_a_flag() {
	return command_flags & A_FLAG_MASK;
}

//--------------------------------------------
// FUNCTION: is_any_flag
// returns whether any has been chosen in the invocation of the program through bitwise AND-ing the mask for every flag
// PARAMETERS:
// none/null/nil, whatever your preference
//----------------------------------------------
int is_any_flag() {
	return command_flags & ALL_FLAG_MASK;
}

//--------------------------------------------
// FUNCTION: is_no_arguments
// returns whether no arguments have been given in the invocation of the program through bitwise AND-ing the mask for no arguments
// PARAMETERS:
// none/null/nil, whatever your preference
//----------------------------------------------
int is_no_arguments() {
	return command_flags & NO_ARG_MASK;
}

//--------------------------------------------
// FUNCTION: is_not_hidden_file_or_can_access
// returns whether the file/directory is not a hidden one or if it is, that it can be accessed and it isn't either '.' or '..'
// PARAMETERS:
// char* file_name - the name of the file/directory
//----------------------------------------------
int is_not_hidden_file_or_can_access(char* file_name) {
	return *file_name != '.' || (*file_name == '.' && is_a_flag() && strlen(file_name) > 2);
	// strlen(file_name) > 2 covers '..' and '.'
}

//--------------------------------------------
// FUNCTION: get_dir_block_count
// calculates the total block count by summing the number of 512B blocks and then halving them by two
// PARAMETERS:
// char** dir_entries_names - String array holding all the names of the directory/file entries for a given directory
// int file_count - total file count in a given directory (length of dir_entries_names)
//----------------------------------------------
int get_dir_block_count(char** dir_entries_names, int file_count) { // used in -l for block count
	int total = 0;
	for(int i = 0; i < file_count; i++) {
		struct stat d_st;
		stat(dir_entries_names[i], &d_st);
		total += (int) d_st.st_blocks;
	}
	return total / 2;
}

//--------------------------------------------
// FUNCTION: get_file_permissions
// determines a given file/directory's permissions and returns them parsed as a string.
// PARAMETERS:
// mode_t mode - mode_t struct holding the file permissions and type; passed in as st_mode property from stat struct
// int is_dir - whether the passed in file/directory is, well, a directory or not.
//----------------------------------------------
char* get_file_permissions(mode_t mode, int is_dir) {
	// -------------------------------------
	// init permissions arr
	char* permissions = malloc(sizeof(char) * PERMISSIONS_SIZE);
	int perm_idx = 0; 
	// -------------------------------------
		
	mode &= ~S_IFMT; // AND out the unnecessary bits and get those needed for file permission (by NOT-in the macro)
	
	// -------------------------------------
	// add permissions
	permissions[perm_idx++] = is_dir ? 'd' : '-';
	permissions[perm_idx++] = mode & S_IRUSR ? 'r' : '-'; // Goddamnit C,
	permissions[perm_idx++] = mode & S_IWUSR ? 'w' : '-'; // do you even have
	permissions[perm_idx++] = mode & S_IXUSR ? 'x' : '-'; // dictionaries/maps???
	permissions[perm_idx++] = mode & S_IRGRP ? 'r' : '-'; // And no,
	permissions[perm_idx++] = mode & S_IWGRP ? 'w' : '-'; // I am not making my own
	permissions[perm_idx++] = mode & S_IXGRP ? 'x' : '-';
	permissions[perm_idx++] = mode & S_IROTH ? 'r' : '-'; // also, I know just printing them
	permissions[perm_idx++] = mode & S_IWOTH ? 'w' : '-'; // is far smarter
	permissions[perm_idx++] = mode & S_IXOTH ? 'x' : '-'; // but I guess I'm just a masochist who writes bad code
	
	permissions[perm_idx] = '\0';
	// -------------------------------------
	
	return permissions;
}

//--------------------------------------------
// FUNCTION: get_file_prefix
// determines the prefix for a given file
// PARAMETERS:
// mode_t mode - mode_t struct holding the file permissions and type; passed in as st_mode property from stat struct
//----------------------------------------------
char get_file_prefix(mode_t mode) {
	if(S_ISREG(mode)) {
		return '-';
	}
	if(S_ISDIR(mode)) {
		return 'd';
	}
	if(S_ISSOCK(mode)) {
		return 's';
	}
	if(S_ISLNK(mode)) {
		return 'l';
	}
	if(S_ISBLK(mode)) {
		return 'b';
	}
	if(S_ISCHR(mode)) {
		return 'c';
	}
	if(S_ISFIFO(mode)) {
		return 'p';
	}
	return ' ';
}

//--------------------------------------------
// FUNCTION: ls_file_l
// handles the calling of ls with the -l flag on a file/directory (both work)
// PARAMETERS:
// struct stat* st - stat struct holding necessary information for a file needed in -l; gained through the stat() call
// int is_dir - whether the passed in file/directory is, well, a directory or not.
// char* name - the file name; used for display in -l
//----------------------------------------------
void ls_file_l(struct stat* st, int is_dir, char* file_name) {
	// check forbidden files
	// implement logic here w/ everything
	// get permissions, deep links, users/groups, time w/ localtime(), file/dir name
	
	// -------------------------------------
	// get permissions, users, and time
	char* permissions = get_file_permissions(st->st_mode, is_dir);
	
	struct passwd* u_pwd; // storing user info
	struct group* g_pwd; // storing group info
	
	u_pwd = getpwuid(st->st_uid);
	g_pwd = getgrgid(st->st_gid);
	
	struct tm* t = localtime(&st->st_mtim); // access time
	char time[256];
	strftime(time, sizeof(time), "%b %e %H:%M", t);
	// -------------------------------------
	
	
	// -------------------------------------
	// print -l info
	printf("%s %d %s %s %ld %s %s\n", permissions, (unsigned) st->st_nlink, u_pwd->pw_name, g_pwd->gr_name, st->st_size, time, file_name);
	// -------------------------------------
	
	free(permissions);
}

//--------------------------------------------
// FUNCTION: ls_default
// handles the default ls call on both files and directories
// PARAMETERS:
// char* file_name - the file name; used for display in ls, the stat() call, and garnered through dirent struct
// struct stat st - stat struct holding necessary information for a file; gained through the stat() call
// int is_just_file - whether the passed in file/directory is just a file or a directory; used for resolving spacing
//----------------------------------------------
void ls_default(char* file_name, struct stat file_stat, int is_just_file) { // handles both -A and default, along with dirs and files
	// -------------------------------------
	if(idx > optind && is_just_file) printf("\n"); // check for newspace
	printf("%c %s\n", get_file_prefix(file_stat.st_mode), file_name); // print default info
	// -------------------------------------
}

//--------------------------------------------
// FUNCTION: ls_file
// handles the ls call on files
// PARAMETERS:
// struct stat* st - stat struct holding necessary information for the given file; gained through the stat() call
// char* path - the file name/path; used for display in ls and garnered through dirent struct
//----------------------------------------------
void ls_file(struct stat* st, char* path) {
	// -------------------------------------
	// determine proper ls for file
	if(is_l_flag()) {
		ls_file_l(st, 0, path);
	} else {
		ls_default(path, *st, 1);
	}
	// -------------------------------------
}

void ls(char*, int); // declare func here as to use it for recursion in ls_dir_r

//--------------------------------------------
// FUNCTION: ls_dir_r
// handles the recursive ls call on directories (and, therein, files as well)
// PARAMETERS:
// char* dir_name - the original directory name passed in for a recursive call
// char* original_path - the original source file name/path; used for creating the new path
//----------------------------------------------
void ls_dir_r(char* dir_name, char* original_path) {
	// -------------------------------------
	// Derive the new path from the original and the new dir name
	char new_path[PATH_MAX];
	int new_path_len = original_path[strlen(original_path) - 1] == '/' ? snprintf(new_path, sizeof(new_path) - 1, "%s%s", original_path, dir_name) : snprintf(new_path, sizeof(new_path) - 1, "%s/%s", original_path, dir_name);
	// -------------------------------------

	ls(new_path, 1); // recursive call to ls_dir()
}

//--------------------------------------------
// FUNCTION: ls_dir_l
// handles the recursive ls call on directories (and, therein, files as well)
// PARAMETERS:
// char* file_name - the file name; used for display in ls, the stat() call, and garnered through dirent struct
// char* original_path - the original source file name/path; used for creating the new path
//----------------------------------------------
void ls_dir_l(char* file_name, struct stat file_stat) {
	// -------------------------------------
	// check if dir because of the possibility of files and dirs inside dirs
	ls_file_l(&file_stat, S_ISDIR(file_stat.st_mode), file_name);
	// -------------------------------------
}

//--------------------------------------------
// FUNCTION: ls_dir
// handles the ls call on directories and the files/directories within the directories if needed
// PARAMETERS:
// struct stat* st - stat struct holding necessary information for the given directory; gained through the stat() call
// char* path - the original source file name/path passed in from the arguments or recursion
// int is_recursion - flag to state whether the current invocation of ls_dir is of a recursive nature (used in spacing, again).
//----------------------------------------------
void ls_dir(struct stat* st, char* path, int is_recursion) {
	// -------------------------------------
	// initialize dir and open it
	DIR* dir;
	
	errno = 0;
	if((dir = opendir(path)) == NULL && errno == EACCES) {
		write_no_access_directory_error(path);
		return;
	} // error check here
	// -------------------------------------
	
	
	// -------------------------------------
	// change dir if needed
	if(strcmp(path, ".") != 0) {
		if(chdir(path) != 0) { // changes the current working directory to the one set by ls 
			write_no_access_directory_error(path);
			return;
		}
	}
	// -------------------------------------


	// -------------------------------------
	struct dirent* dir_entry;
	
	char** dir_entries_names = (char**) malloc(sizeof(char*)); // array to keep track of all the file names

	int file_count = 0; // total file count
	// -------------------------------------
	
	
	// -------------------------------------
	// handle directory prefix
	if(!(is_no_arguments()) || is_r_flag()) {
		if(idx > optind || is_recursion) printf("\n");
		printf("%s:\n", path); 
	}
	// if there is at least one argument, call the prefix
	// -R lists it out no matter what
	// -------------------------------------
	
	
	// -------------------------------------
	// read the directory
	while((dir_entry = readdir(dir)) != NULL) {
		// check for hidden files flag and don't check later on
		if(is_not_hidden_file_or_can_access(dir_entry->d_name)) {
			dir_entries_names[file_count] = malloc(strlen(dir_entry->d_name) + 1); // +1 for terminating null char
			strcpy(dir_entries_names[file_count++], dir_entry->d_name);
			dir_entries_names = (char**) realloc(dir_entries_names, PATH_MAX); // reallocate w/ PATH_MAX space for new str
		}
	}
	// -------------------------------------
	
	
	// -------------------------------------
	// declare directory idx array
	int* directories_indices = (int*) malloc(sizeof(int) * file_count); // array to keep track of all the indices of directories
	int dir_count = 0; // total directory count
	// -------------------------------------
	
	
	// -------------------------------------
	// calculate total for -l
	if(is_l_flag()) {
		int total = get_dir_block_count(dir_entries_names, file_count); // get the block count and pass it on for -l
		printf("total %d\n", total);	
	}
	// -------------------------------------
	
	
	// -------------------------------------
	// call ls()-es
	for(int i = 0; i < file_count; i++) {
		struct stat file_stat; // current directory stat struct

		if(stat(dir_entries_names[i], &file_stat) == -1) {
			continue;
		}
		
		if(is_r_flag() && S_ISDIR(file_stat.st_mode) && *dir_entries_names[i] != '.') {
			directories_indices[dir_count++] = i;
			// add to directories array if -R flag is enabled
		}
		
		if(is_l_flag()) {
			ls_dir_l(dir_entries_names[i], file_stat);
		} else {
			ls_default(dir_entries_names[i], file_stat, 0); 
			// ls_dir_default() handles both -A, -R, and default 
			// (which is why check is like that and doesn't use ALL_FLAG_MASK)
		}
	}
	// -------------------------------------
	
	
	// -------------------------------------
	// Handle -R
	if(is_r_flag()) {
		// change back to the source dir as to change again in the inner directories
		if(chdir(source_dir) != 0) {
			perror("");
		}
		for(int i = 0; i < dir_count; i++) {
			ls_dir_r(dir_entries_names[directories_indices[i]], path); 
			// call the recursive ls at the end of this if the flag is set
		}
	}
	// -------------------------------------
	
	
	// -------------------------------------
	// Free the memory taken
	for(int i = 0; i < file_count; i++) {
		free(dir_entries_names[i]);
	}
	
	free(dir_entries_names);
	free(directories_indices);
	// -------------------------------------


	// -------------------------------------
	// change back to the source directory in case of switch and close the directory
	if(chdir(source_dir) != 0) {
		perror("");
	}
	
	closedir(dir); // close dir
	// -------------------------------------
}

//--------------------------------------------
// FUNCTION: ls
// handles the ls call on arguments and determines order of execution further onwards
// PARAMETERS:
// char* path - the original source file name/path passed in from the command-line arguments or recursion
// int is_recursion - flag to state whether the current invocation of ls is of a recursive nature (used in ls_dir() and spacing, again).
//----------------------------------------------
void ls(char* path, int is_recursion) {
	// determines apt functions by calling stat() and ls_type_arg if needed
	// 1 - for directories; 0 - files
	
	// -------------------------------------
	// pass in the given path to stat
	struct stat st;
	errno = 0;
	if(stat(path, &st) == -1  && errno == ENOENT) { // switch to OR from AND?
		write_not_exist_file_error(path);
		return;
	} // error check here
	// -------------------------------------


	// -------------------------------------
	// check if the result from stat is a dir
	if(S_ISDIR(st.st_mode)) {
		ls_dir(&st, path, is_recursion);
	} else ls_file(&st, path);
	// -------------------------------------
}

//--------------------------------------------
// FUNCTION: main
// handles the user input w/ command-line args and getopt() for optional flags 
// PARAMETERS:
// int argc - the count of the command-line arguments; or the length of the argv string array
// char** argv - the argument values stored in a string array
//----------------------------------------------
int main(int argc, char** argv) {
	int opt;
	
	// -------------------------------------
	// Get the user's current working directory as the src and their default locale
	getcwd(source_dir, sizeof(source_dir)); // get the default current working directory and save it for later use
	setlocale(LC_ALL, ""); // set the default language to the user's default
	// -------------------------------------
	
	
	// -------------------------------------
	// read opt args
	while((opt = getopt(argc, argv, OPT_LIST)) != -1) { // -1 -> no more optional flags
		switch(opt) {
			case 'A':
				command_flags |= A_FLAG_MASK; // add the -A flag mask to the command_flag var
				break;
			case 'l':
				command_flags |= L_FLAG_MASK; // add the -l flag mask to the command_flag var
				break;
			case 'R':
				command_flags |= R_FLAG_MASK; // add the -R flag mask to the command_flag var
				break;
			default: exit(1);
		}
	}
	// -------------------------------------
	
	
	// -------------------------------------
	// Check opt args and determine NO_ARG_MASK flag usage
	if(optind == argc) { // if no actual arguments are passed, just use the default dir
		command_flags |= NO_ARG_MASK;
		ls(DEFAULT_DIR, 0);
		exit(0);
	}
	
	if(optind + 1 == argc) command_flags |= NO_ARG_MASK; 
	// if just one argument is passed, use it w/ the proper formatting
	// -------------------------------------
	
	
	// -------------------------------------
	// In any other case with more than 1 arguments, run them through ls()
	for(idx = optind; idx < argc; idx++) {
	 	// if the passed in dir is ".", pass the cwd first found in main()
	 	// this is required due to changes in directories
		ls(argv[idx], 0);
	}
	// -------------------------------------
	
	exit(0);
}
