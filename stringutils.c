#include "stringutils.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

size_t nullSafeStrLen(const char * string) {
	if(string == NULL) {
		return 0;
	}
	else {
		return strlen(string);
	}
}

bool extractBoolQueryParam(const struct mg_request_info *request_info, const char * paramKey) {
	return extractBoolQueryParamDefault(request_info, paramKey, false);
}

bool extractBoolQueryParamDefault(const struct mg_request_info *request_info, const char * paramKey, const bool def) {
	char queryParam[10];
	size_t queryLength = nullSafeStrLen(request_info->query_string);
	int res = mg_get_var(request_info->query_string, queryLength, paramKey, queryParam, 10);
	if(res > 0) {
		return (queryParam[0] == '1');
	}
	return def;
}

int extractStringQueryParam(const struct mg_request_info *request_info, const char * paramKey, char * buffer, const int length) {
	size_t queryLength = nullSafeStrLen(request_info->query_string);

	return mg_get_var(request_info->query_string, queryLength, paramKey, buffer, length);
}

int extractStringQueryParamDefault(const struct mg_request_info *request_info, const char * paramKey, const char * def, char * buffer, const int length) {
	int res = extractStringQueryParam(request_info, paramKey, buffer, length);
	
	if( res < 0 ) {
		strncpy(buffer, def, length);
		return strlen(def);
	}
	return res;
}

int extractIntQueryParam(const struct mg_request_info *request_info, const char *paramKey) {
	char buffer[20];
	int res = extractStringQueryParam(request_info, paramKey, buffer, 20);
	if( res < 0 ) {
		return 0;
	}
	if(isInteger(buffer)) {
		return atoi(buffer);
	}
	else {
		return 0;
	}
}

bool isInteger(const char * s)
{
   if(s == NULL || *s == '\0') return false ;

   char * p ;
   strtol(s, &p, 10) ;

   return (*p == 0) ;
}