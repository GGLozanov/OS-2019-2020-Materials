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

static short command_flags = 0b0000;

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

void get_dir_block_count(DIR* dir, int* total) { // used in -l for block count
	struct dirent* dir_entry;
	while((dir_entry = readdir(dir)) != NULL) {
		struct stat d_st;
		stat(dir_entry->d_name, &d_st);
		*total += (int) d_st.st_blocks;
	}
	*total /= 2;
}

int is_hidden_file_or_can_access(struct dirent* dir_entry) {
	return *dir_entry->d_name != '.' || (*dir_entry->d_name == '.' && command_flags & A_FLAG_MASK);
}

char* get_file_permissions(struct stat st, int is_dir) {
	char* permissions = malloc(sizeof(char) * PERMISSIONS_SIZE);
	mode_t _mode = st.st_mode & ~S_IFMT; // AND out the unnecessary bits and get those needed for file permission (by NOT-in the macro)
	// lots of ifs...
	int perm_idx = 1;
	
	*permissions = is_dir ? 'd' : '-';
	
	permissions[perm_idx++] = _mode & S_IRUSR ? 'r' : '-';
	permissions[perm_idx++] = _mode & S_IWUSR ? 'w' : '-';
	permissions[perm_idx++] = _mode & S_IXUSR ? 'x' : '-';
	permissions[perm_idx++] = _mode & S_IRGRP ? 'r' : '-';
	permissions[perm_idx++] = _mode & S_IWGRP ? 'w' : '-';
	permissions[perm_idx++] = _mode & S_IXGRP ? 'x' : '-';
	permissions[perm_idx++] = _mode & S_IROTH ? 'r' : '-';
	permissions[perm_idx++] = _mode & S_IWOTH ? 'w' : '-';
	permissions[perm_idx++] = _mode & S_IXOTH ? 'x' : '-';
	
	return permissions;
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
	return NULL;
}

void ls_file_r(struct stat* st, char* name) {
	// leave the recursive calls for the directory and have it handle; just read file normally
	// probably don't need this function
	// yep, just handle with default ls_file
}

void ls_file_l(struct stat* st, int is_dir, char* name) {
	// check forbidden files
	// implement logic here w/ everything
	// get permissions, deep links, users/groups, time w/ localtime(), file/dir name
	// ternary for permissions
	char* permissions = get_file_permissions(*st, is_dir);
	
	struct passwd* u_pwd; // storing user info
	struct passwd* g_pwd; // storing group info
	struct tm* t = localtime(&st->st_mtim);
	char time[256];
	strftime(time, sizeof(time), "%b %e %H:%M", t);
	// TODO: Find way to omit day of the week	
	
	u_pwd = getpwuid(st->st_uid);
	g_pwd = getgrgid(st->st_gid);
	
	printf("%s %d %s %s %ld %s %s\n", permissions, (unsigned) st->st_nlink, u_pwd->pw_name, g_pwd->pw_name, st->st_size, time, name);
	
	free(permissions);
}

void ls_file_default(struct stat* st, char* name) {
	printf("%c %s\n", get_file_prefix(*st), name);
}

void ls_file(struct stat* st, char* path) {
	if(command_flags & R_FLAG_MASK) ls_file_r(st, path);			
	else if(command_flags & L_FLAG_MASK) ls_file_l(st, 0, path);
	else if(!(command_flags & ALL_FLAG_MASK) || command_flags & A_FLAG_MASK) {
		ls_file_default(st, path);
	}
}

void ls_dir_a(struct dirent* dir_entry) {
	// call ls_dir_default() w/ mode for hidden files set to true
}

void ls_dir_r(struct dirent* dir_entry) {
	// recursive call to ls()
}

void ls_dir_l(struct dirent* dir_entry, int total) { // pass in copy of direntry, not ptr? Teacher told us to copy our direntries
	// check NO_ARG_MASK here
	if(is_hidden_file_or_can_access(dir_entry)) {
		struct stat dir_stat;
		stat(dir_entry->d_name, &dir_stat);
		ls_file_l(&dir_stat, S_ISDIR(dir_stat.st_mode), dir_entry->d_name); // ternary for 1 and 0?
	}
}

void ls_dir_default(struct dirent* dir_entry) { // handles both -A and default
	if(is_hidden_file_or_can_access(dir_entry)) {
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
	
	int total = 0; 
		
	if(!(command_flags & NO_ARG_MASK)) printf("%s:\n", path); 
	// if there is at least one argument, call the prefix
	// -l also has the listing apparently -> !(command_flags & L_FLAG_MASK) && 
	if(command_flags & L_FLAG_MASK) {
		// TODO: 100% find a goddamn way to optimise this or I swear to god
		get_dir_block_count(dir, &total);
		printf("total %d\n", total);
		closedir(dir);
		if((dir = opendir(path)) == NULL) {
			write_no_access_directory_error(path);
			return;
		} // error check here
	}
	
	while((dir_entry = readdir(dir)) != NULL) {
	    // optimise bool condition
	    	if(command_flags & R_FLAG_MASK) ls_dir_r(dir_entry);		
		else if(command_flags & L_FLAG_MASK) ls_dir_l(dir_entry, total);
		else if(!(command_flags & ALL_FLAG_MASK) || command_flags & A_FLAG_MASK) {
			ls_dir_default(dir_entry); 
			// ls_dir_default() handles both -A and default 
			// (which is why check is like that and doesn't use ALL_FLAG_MASK)
		}
	}

	closedir(dir);
}

void ls(char* path) {
	// determines apt functions by calling stat() and ls_type_arg if needed
	// 1 - for directories; 0 - files
	struct stat st;
	if(stat(path, &st) == -1  && errno == ENOENT) { // switch to OR from AND?
		write_not_exist_file_error(path);
		return;
	} // error check here

	// might need to move AND to prefix function

	
	if(S_ISDIR(st.st_mode)) {
		ls_dir(&st, path);
	} else ls_file(&st, path);

}

int main(int argc, char** argv) {
	int opt;
	int idx;
	// char** opt_args;
	setlocale(LC_ALL, "");
	
	while((opt = getopt(argc, argv, OPT_LIST)) != -1) {
		switch(opt) {
			case 'A':
				// printf("Received -A as arg!\n");
				// printf("arg: %s\n", optarg);
				command_flags |= A_FLAG_MASK; // add the -A flag mask to the command_flag var
				break;
			case 'l':
				// printf("Received -l as arg!\n");
				// printf("arg: %s\n", optarg);
				command_flags |= L_FLAG_MASK; // add the -l flag mask to the command_flag var
				break;
			case 'R':
				// printf("Received -R as arg!\n");
				// printf("arg: %s\n", optarg);
				command_flags |= R_FLAG_MASK; // add the -R flag mask to the command_flag var
				break;
			default: exit(1);
		}
	}
	
	// set the opt index back to the start
	// cond -> *argv[optind] != '-' ?
	if(optind == argc) { // if no actual arguments are passed, just use the default dir
		command_flags |= NO_ARG_MASK;
		ls(DEFAULT_DIR);
		exit(0);
	}
	
	if(optind + 1 == argc) command_flags |= NO_ARG_MASK; // if just one argument is passed, use it w/ the proper formatting
	
	for(idx = optind; idx < argc; idx++) {
		ls(argv[idx]);
		if(!(command_flags & NO_ARG_MASK) && idx == argc) printf("\n"); 
	}
	// run stat() and check if dir => do stuff depending on type
}
