#ifndef __STRING_UTILS_H
#define __STRING_UTILS_H

#include <stdlib.h>
#include <stdbool.h>

#include "mongoose.h"

size_t nullSafeStrLen(const char * string);

bool extractBoolQueryParam(const struct mg_request_info *request_info, const char *paramKey);
bool extractBoolQueryParamDefault(const struct mg_request_info *request_info, const char *paramKey, const bool def);

int extractStringQueryParam(const struct mg_request_info *request_info, const char *paramKey, char *buffer, const int length);
int extractStringQueryParamDefault(const struct mg_request_info *request_info, const char *paramKey, const char *def, char *buffer, const int length);

#endif