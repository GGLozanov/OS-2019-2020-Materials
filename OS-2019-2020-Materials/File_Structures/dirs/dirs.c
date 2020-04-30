#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>

int main() {
	// opendir, readdir, closedir
	// readdir check for null and error (errno set to 0 before check w/readdir)
	// readdir returns directory structure
	DIR* dirptr = opendir("../ls/");
	struct dirent* direntry; // readdir can't be used forever (result can be overwritten, so you need to copy your info)
	while(direntry = readdir(dirptr)) {
		printf("%s\n", direntry->d_name); // need to copy information of the struct in another object (translation method from ptr to obj)
	}
	
	closedir(dirptr);
	
	struct stat st;
	stat("../ls", &st);
	printf("%d\n", S_ISDIR(st.st_mode));
}
