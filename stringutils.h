#ifndef __STRING_UTILS_H
#define __STRING_UTILS_H

#include <stdlib.h>
#include <stdbool.h>

#include "mongoose.h"

size_t n_strlen(const char * string);

bool bool_query_param(const struct mg_request_info *request_info, const char *param);
bool bool_query_param_def(const struct mg_request_info *request_info, const char *param, const bool def);

int str_query_param(const struct mg_request_info *request_info, const char *param, char *buffer, const int length);
int str_query_param_def(const struct mg_request_info *request_info, const char *param, const char *def, char *buffer, const int length);

// this one always returns zero as a default value.
int int_query_param(const struct mg_request_info *request_info, const char *param);

bool is_int(const char * s);

// returns true if the given string s contains / or .. 
bool contains_path_chars(const char * s);

void indent(FILE * outputStream, int depth);

#endif