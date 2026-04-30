#include "utils.h"
#include <ctype.h>
#include <string.h>

/* Convert a string to uppercase
 */
char* strtoupper(char *str)
{
	char *ptr = str;

	while(*ptr) {
		(*ptr) = toupper(*ptr);
		ptr++;
	}
	return str;
}

/* Trim whitespace and newlines from a string
 */
char* trim(char *str)
{
	char *pch = str;
	while(isspace(*pch)) {
		pch++;
	}
	if(pch != str) {
		memmove(str, pch, (strlen(pch) + 1));
	}

	size_t len = strlen(str);
	if(len == 0) {
		return str;
	}

	pch = (char*)(str + (len - 1));
	while(isspace(*pch)) {
		pch--;
	}
	*++pch = '\0';

	return str;
}