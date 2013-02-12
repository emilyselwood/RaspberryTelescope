
#ifndef __FILE_UTILS_H
#define __FILE_UTILS_H

#include <stdio.h>

/**
 * create a json blob listing the contence of the file
 */
int listImageDirectory(const char * path, FILE * outputStream);

#endif