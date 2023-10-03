#include <libcircles/util.h>
#include <stdio.h>
#include <stdlib.h>

// Not going to hide this file from user.
// Functions from here might still be useful to somebody

unsigned int circles_uleb128_decode(int size, char* bytes) { // Stolen from https://en.wikipedia.org/wiki/LEB128
	int result = 0;
	int shift = 0;
	for(int i = 0; i < size; i++) {
 		result |= (bytes[i] & 127) << shift;
  		if ((bytes[i] & 128) == 0)
    		break;

		shift += 7;
	}

	return result;
}

int circles_fpstring_parse(char** ptr, CirclesCallbackRead callback, void* ctx) {
	char temp;
	size_t temp_size;
	char bytes[16];

	temp_size = 1;
	if((*callback)(ctx, &temp, &temp_size) || temp_size != 1)
		return CIRCLES_ERROR_BROKEN_STREAM;

	if(temp != 11) {
		*ptr = NULL;
		return 0;
	}

	int i = 0;
	do {
		temp_size = 1;
		if((*callback)(ctx, &temp, &temp_size) || temp_size != 1)
			return CIRCLES_ERROR_BROKEN_STREAM;
		bytes[i] = temp;
		i++;
	} while((temp & 128) != 0 && i < 16);

	unsigned int len = circles_uleb128_decode(i, bytes);
	*ptr = (char*) malloc(len + 1);
	if(*ptr == NULL)
		return CIRCLES_ERROR_ALLOC_FAILED;

	temp_size = len;
	if((*callback)(ctx, *ptr, &temp_size) || temp_size != len)
		return CIRCLES_ERROR_BROKEN_STREAM;

	(*ptr)[len] = 0;

	return 0;
}

long long circles_jesustime_to_unixms(long long ticks) { // Jesustime is a fancy name for Windows ticks
	return (ticks - 621355968000000000 /* unix epoch start in ticks */) / 10000 /* Windows tick is 100 nanoseconds but for whatever reason we have to multiply it by 100 */;
}
