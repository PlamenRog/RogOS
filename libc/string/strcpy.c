#include <string.h>

char* strcpy(char* dest, const char* src) {
	char* start = dest;

	while ((*dest++ = *src++))
		;

	return start;
}
