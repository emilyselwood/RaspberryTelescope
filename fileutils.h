
#ifndef __FILE_UTILS_H
#define __FILE_UTILS_H

#include <stdio.h>
#include <stdbool.h>

/**
 * create a json blob listing the contence of the file
 */
int list_img_dir(const char * path, FILE * output);

/**
 * Checks for the existence of a directory. Returns true if the directory exists and the path actually points to a directory
 */
bool dir_exits(const char * path);

/**
 * Checks for the existence of a file. Returns true only if the path exists and it is a file.
 */
bool file_exists(const char * path);

#endif