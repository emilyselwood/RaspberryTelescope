#include "stringutils.h"
#include <stdlib.h>
#include <string.h>

size_t nullSafeStrLen(const char * string) {
	if(string == NULL) {
		return 0;
	}
	else {
		return strlen(string);
	}
}