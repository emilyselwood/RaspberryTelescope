#include "fileutils.h"
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>

#include "stringutils.h"


void print_entry(FILE * output, const char * key, const char * value) {
	indent(output, 3);
	fprintf(output, "\"%s\" : \"%s\",\n", key, value);
}

void print_int_entry(FILE * output, const char * key, const int value) {
	indent(output, 3);
	fprintf(output, "\"%s\" : \"%d\",\n", key, value);
}

int list_img_dir(const char * path, FILE * output) {

	DIR *dir;

	if ((dir = opendir (path)) != NULL) {
		
		fprintf(output, "{\n\t\"files\" : [\n");
		struct dirent *ent;
		int pathLength = strlen(path);
		bool first = true;
		while ((ent = readdir (dir)) != NULL) {
			// ignore hidden files.
			if(ent->d_name[0] != '.') {
				
				struct stat fileStat;
				int length = strlen(ent->d_name) + pathLength + 2;
				char * fullname = (char*) malloc(sizeof(char) * length);
				snprintf(fullname, length, "%s/%s", path, ent->d_name);
				if(stat(fullname, &fileStat) >= 0) {
					if(first) {
						first = false;
					}
					else {
						fprintf(output, ",\n");
					}
					indent(output, 2);
					fprintf(output, "{\n");
					print_entry(output, "name", ent->d_name);
					print_int_entry(output, "size", (int)fileStat.st_size);
					print_int_entry(output, "date", (int)fileStat.st_mtime);
					indent(output, 2);
					fprintf(output, "}");
				}
				free(fullname);
			}
		}
		closedir (dir);
		fprintf(output, "\n\t]\n}\n");
		fflush(output);
		return 0;
	}
	else {
		fprintf(stderr, "Could not open directory %s\n", path);
		return -1;
	}
}


bool dir_exits(const char * path) {
	struct stat st;
	if(stat(path,&st) != 0) {
		return false;
	}
	
	if(S_ISDIR(st.st_mode)) {
		return true;
	}
	else {
		return false;
	}
}

bool file_exists(const char * path) {
	struct stat st;
	if(stat(path,&st) != 0) {
		return false;
	}
	
	if(S_ISREG(st.st_mode)) {
		return true;
	}
	else {
		return false;
	}
}

