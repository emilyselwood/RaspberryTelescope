#include "stringutils.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>

size_t n_strlen(const char * string) {
	if(string == NULL) {
		return 0;
	}
	else {
		return strlen(string);
	}
}

bool bool_query_param(const struct mg_request_info *request_info, const char * param) {
	return bool_query_param_def(request_info, param, false);
}

bool bool_query_param_def(const struct mg_request_info *request_info, const char * param, const bool def) {
	char queryParam[10];
	size_t queryLength = n_strlen(request_info->query_string);
	int res = mg_get_var(request_info->query_string, queryLength, param, queryParam, 10);
	if(res > 0) {
		return (queryParam[0] == '1');
	}
	return def;
}

int str_query_param(const struct mg_request_info *request_info, const char * param, char * buffer, const int length) {
	size_t queryLength = n_strlen(request_info->query_string);

	return mg_get_var(request_info->query_string, queryLength, param, buffer, length);
}

int str_query_param_def(const struct mg_request_info *request_info, const char * param, const char * def, char * buffer, const int length) {
	int res = str_query_param(request_info, param, buffer, length);
	if( res < 0 ) {
		strncpy(buffer, def, length);
		return strlen(def);
	}
	return res;
}

int int_query_param(const struct mg_request_info *request_info, const char *param) {
	char buffer[20];
	int res = str_query_param(request_info, param, buffer, 20);
	if( res < 0 ) {
		return 0;
	}
	if(is_int(buffer)) {
		return atoi(buffer);
	}
	else {
		return 0;
	}
}

bool is_int(const char * s) {
	if(s == NULL || *s == '\0') {
		return false;
	}

	char * p ;
	long res = strtol(s, &p, 10) ;
	if((res == LONG_MAX || res == LONG_MIN ) && errno == ERANGE) {
		return false;
	}

	return (*p == 0) ;
}

bool contains_path_chars(const char * s) {
	int len = n_strlen(s);
	for(int i = 0; i < len; i++) {
		if(s[i] == '/') {
			return true;
		}
		if(s[i] == '.' && s[i+1] == '.') {
			return true;
		}
	}
	return false;
}

void indent(FILE * outputStream, const int depth) {
	for( int i = 0; i < depth; i++ ) {
		fprintf(outputStream, "\t");
	}
}