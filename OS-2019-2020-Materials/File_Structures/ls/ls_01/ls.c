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
#define ALL_FLAG_MASK (A_FLAG_MASK | L_FLAG_MASK | R_FLAG_MASK) // 0b111
#define OPT_LIST "ARl" // options list
#define DEFAULT_DIR "."
#define PERMISSIONS_SIZE 11

// TODO: Implement -R and error checks to continue if error in stat() is encountered
// TODO: Fix spacing

static short command_flags = 0b0000;
int idx; // optind idx (follows through the program up to argc)
char source_dir[PATH_MAX]; // source directory wherein ls is first called

void write_not_exist_file_error(char* filename) {
    write(STDERR_FILENO, "ls: cannot access ", 18);
    write(STDERR_FILENO, filename, strlen(filename));
    write(STDERR_FILENO, ": ", 2);
    perror("");
}

void write_no_access_directory_error(char* dirname) {
    write(STDERR_FILENO, "ls: cannot open directory ", 26);
    write(STDERR_FILENO, dirname, strlen(dirname));
    write(STDERR_FILENO, ": ", 2);
    perror("");
}

int get_dir_block_count(char** dir_entries_names, int file_count) { // used in -l for block count
	int total = 0;
	for(int i = 0; i < file_count; i++) {
		struct stat d_st;
		stat(dir_entries_names[i], &d_st);
		if(d_st.st_blocks != NULL) total += (int) d_st.st_blocks;
	}
	return total / 2;
}

int is_not_hidden_file_or_can_access(char* file_name) {
	return *file_name != '.' || (*file_name == '.' && command_flags & A_FLAG_MASK && strlen(file_name) > 2);
	// strlen(file_name) > 2 covers '..' and '.'
}

char* get_file_permissions(mode_t mode, int is_dir) {
	char* permissions = malloc(sizeof(char) * PERMISSIONS_SIZE);
	
	mode &= ~S_IFMT; // AND out the unnecessary bits and get those needed for file permission (by NOT-in the macro)
	
	int perm_idx = 0; 
	
	permissions[perm_idx++] = is_dir ? 'd' : '-';
	permissions[perm_idx++] = mode & S_IRUSR ? 'r' : '-'; // Goddamnit C,
	permissions[perm_idx++] = mode & S_IWUSR ? 'w' : '-'; // do you even have
	permissions[perm_idx++] = mode & S_IXUSR ? 'x' : '-'; // dictionaries/maps???
	permissions[perm_idx++] = mode & S_IRGRP ? 'r' : '-'; // and no,
	permissions[perm_idx++] = mode & S_IWGRP ? 'w' : '-'; // I am not making my own
	permissions[perm_idx++] = mode & S_IXGRP ? 'x' : '-';
	permissions[perm_idx++] = mode & S_IROTH ? 'r' : '-'; // also, I know just printing them
	permissions[perm_idx++] = mode & S_IWOTH ? 'w' : '-'; // is far smarter
	permissions[perm_idx++] = mode & S_IXOTH ? 'x' : '-'; // but I guess I'm just a masochist who writes bad code
	
	permissions[perm_idx] = '\0';
	
	return permissions;
}

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

void ls_file_l(struct stat* st, int is_dir, char* name) {
	// check forbidden files
	// implement logic here w/ everything
	// get permissions, deep links, users/groups, time w/ localtime(), file/dir name
	// ternary for permissions
	char* permissions = get_file_permissions(st->st_mode, is_dir);
	
	struct passwd* u_pwd; // storing user info
	struct passwd* g_pwd; // storing group info
	
	u_pwd = getpwuid(st->st_uid);
	g_pwd = getgrgid(st->st_gid);
	
	struct tm* t = localtime(&st->st_mtim); // access time
	char time[256];
	strftime(time, sizeof(time), "%b %e %H:%M", t);
	
	printf("%s %d %s %s %ld %s %s\n", permissions, (unsigned) st->st_nlink, u_pwd->pw_name, g_pwd->pw_name, st->st_size, time, name);
	
	free(permissions);
}

void ls_default(char* file_name, struct stat file_stat) { // handles both -A and default, along with dirs and files
	printf("%c %s\n", get_file_prefix(file_stat.st_mode), file_name);
}

void ls_file(struct stat* st, char* path) {
	if(command_flags & L_FLAG_MASK) ls_file_l(st, 0, path);
	else if(!(command_flags & ALL_FLAG_MASK) || command_flags & A_FLAG_MASK || command_flags & R_FLAG_MASK) {
		ls_default(path, *st);
	}
}

void ls_dir_r(char* dir_name, char* original_path) {
	char new_path[PATH_MAX];
	int new_path_len = original_path[strlen(original_path) - 1] == '/' ? snprintf(new_path, sizeof(new_path) - 1, "%s%s", original_path, dir_name) : snprintf(new_path, sizeof(new_path) - 1, "%s/%s", original_path, dir_name);

	ls(new_path, 1); // recursive call to ls_dir()
}

void ls_dir_l(char* file_name, struct stat file_stat) { // pass in copy of direntry, not ptr? Teacher told us to copy our direntries
	// check NO_ARG_MASK here
	// check if dir because of the possibility of files and dirs inside dirs
	ls_file_l(&file_stat, S_ISDIR(file_stat.st_mode), file_name);
}

// also calls ls_file
void ls_dir(struct stat* st, char* path, int is_recursion) {
	DIR* dir;
	
	errno = 0;
	if((dir = opendir(path)) == NULL && errno == EACCES) {
		write_no_access_directory_error(path);
		return;
	} // error check here
		
	if(strcmp(path, ".") != 0) {
		if(chdir(path) != 0) { // changes the current working directory to the one set by ls 
			write_no_access_directory_error(path);
			return;
		}
		// (hey, our teachers didn't mention this function at all and I had to search it up in a SO thread!)
		// oh wait, nevermind, that's just programming in general, isn't it
	}
	
	struct dirent* dir_entry;
	
	char** dir_entries_names = (char**) malloc(sizeof(char*)); // array to keep track of all the file names

	int file_count = 0; // total file count
	
	if(!(command_flags & NO_ARG_MASK) || command_flags & R_FLAG_MASK) {
		if(idx > optind || is_recursion) printf("\n");
		printf("%s:\n", path); 
	}
	// if there is at least one argument, call the prefix
	// -R lists out no 	matter what
	
	while((dir_entry = readdir(dir)) != NULL) { // TODO: readdir() error checks
		// check for hidden files flag and don't check later on
		if(is_not_hidden_file_or_can_access(dir_entry->d_name)) {
			dir_entries_names[file_count] = malloc(strlen(dir_entry->d_name) + 1); // +1 for terminating null char
			strcpy(dir_entries_names[file_count++], dir_entry->d_name);
			dir_entries_names = (char**) realloc(dir_entries_names, PATH_MAX); // reallocate w/ PATH_MAX space for new str
		}
	}
	
	int* directories_indices = (int*) malloc(sizeof(int) * file_count); // array to keep track of all the indices of directories
	int dir_count = 0; // total directory count
	
	if(command_flags & L_FLAG_MASK) {
		int total = get_dir_block_count(dir_entries_names, file_count); // get the block count and pass it on for -l
		printf("total %d\n", total);	
	}
	
	for(int i = 0; i < file_count; i++) {
		// optimise bool condition
		struct stat file_stat; // current directory stat struct

		if(stat(dir_entries_names[i], &file_stat) == -1) {
			continue;
		}
		
		if(command_flags & R_FLAG_MASK && S_ISDIR(file_stat.st_mode) && *dir_entries_names[i] != '.') {
			directories_indices[dir_count++] = i;
			// add to directories array if -R flag is enabled
		}
		
		if(command_flags & L_FLAG_MASK) {
			ls_dir_l(dir_entries_names[i], file_stat);
		} else if(!(command_flags & ALL_FLAG_MASK) || command_flags & A_FLAG_MASK || command_flags & R_FLAG_MASK) {
			ls_default(dir_entries_names[i], file_stat); 
			// ls_dir_default() handles both -A, -R, and default 
			// (which is why check is like that and doesn't use ALL_FLAG_MASK)
		}
	}
	
	if(command_flags & R_FLAG_MASK) {
		if(chdir(source_dir) != 0) {
			perror("");
		}
		for(int i = 0; i < dir_count; i++) {
			ls_dir_r(dir_entries_names[directories_indices[i]], path); // call the recursive ls at the end of this if the flag is set
		}
	}
	
	// ---------------------
	// Free the memory taken
	for(int i = 0; i < file_count; i++) {
		free(dir_entries_names[i]);
	}
	
	free(dir_entries_names);
	free(directories_indices);
	// ---------------------

	if(chdir(source_dir) != 0) {
		perror("");
	} // change back to the source directory in case of switch

	closedir(dir);
}

void ls(char* path, int is_recursion) {
	// determines apt functions by calling stat() and ls_type_arg if needed
	// 1 - for directories; 0 - files
	struct stat st;
	errno = 0;
	if(stat(path, &st) == -1  && errno == ENOENT) { // switch to OR from AND?
		write_not_exist_file_error(path);
		return;
	} // error check here

	// might need to move AND to prefix function

	if(S_ISDIR(st.st_mode)) {
		ls_dir(&st, path, is_recursion);
	} else ls_file(&st, path);
}

int main(int argc, char** argv) {
	int opt;
	
	getcwd(source_dir, sizeof(source_dir)); // get the default current working directory and save it for later use
	setlocale(LC_ALL, ""); // set the default language to the user's default
	
	while((opt = getopt(argc, argv, OPT_LIST)) != -1) {
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
	
	// set the opt index back to the start
	// cond -> *argv[optind] != '-' ?
	if(optind == argc) { // if no actual arguments are passed, just use the default dir
		command_flags |= NO_ARG_MASK;
		ls(DEFAULT_DIR, 0);
		exit(0);
	}
	
	if(optind + 1 == argc) command_flags |= NO_ARG_MASK; // if just one argument is passed, use it w/ the proper formatting
	
	for(idx = optind; idx < argc; idx++) {
	 	// if the passed in dir is ".", pass the cwd first found in main()
	 	// this is required due to changes in directories
		ls(argv[idx], 0);
	}
	
	exit(0);
}
