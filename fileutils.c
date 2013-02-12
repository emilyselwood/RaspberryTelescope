#include "fileutils.h"
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdbool.h>

#include "stringutils.h"

int listImageDirectory(const char * path, FILE * outputStream) {

	DIR *dir;

	if ((dir = opendir (path)) != NULL) {
		
		fprintf(outputStream, "{\n\t\"files\" : [\n");
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
						fprintf(outputStream, ",\n");
					}
					indent(outputStream, 2);
					fprintf(outputStream, "{\n");
					indent(outputStream, 3);
					fprintf(outputStream, "\"name\" : \"%s\",\n", ent->d_name);
					indent(outputStream, 3);
					fprintf(outputStream, "\"size\" : \"%d\",\n", (int)fileStat.st_size);
					indent(outputStream, 3);
					fprintf(outputStream, "\"date\" : \"%d\"\n", (int)fileStat.st_mtime);
					indent(outputStream, 2);
					fprintf(outputStream, "}");
				}
				free(fullname);
			}
		}
		closedir (dir);
		fprintf(outputStream, "\n\t]\n}\n");
		fflush(outputStream);
		return 0;
	}
	else {
		fprintf(stderr, "Could not open directory %s\n", path);
		return -1;
	}
}
