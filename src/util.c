#include <circles/util.h>
#include <stdio.h>
#include <stdlib.h>

uint64_t circles_uleb128_decode(int size, const char* bytes) { // Stolen from https://en.wikipedia.org/wiki/LEB128
	uint64_t result = 0;
	uint64_t shift = 0;
	for(int i = 0; i < size; i++) {
 		result |= (bytes[i] & 127) << shift;
  		if ((bytes[i] & 128) == 0)
    		break;

		shift += 7;
	}

	return result;
}

CirclesError circles_binstring_decode(char** ptr, circles_read_fn_t callback, void* ctx) {
	char temp;
	size_t temp_size;
	char bytes[16];

	temp_size = 1;
	if((*callback)(&temp, &temp_size, ctx) || temp_size != 1)
		return CIRCLES_ERROR_BROKEN_STREAM;

	if(temp != 11) {
		*ptr = NULL;
		return 0;
	}

	int i = 0;
	do {
		temp_size = 1;
		if((*callback)(&temp, &temp_size, ctx) || temp_size != 1)
			return CIRCLES_ERROR_BROKEN_STREAM;
		bytes[i] = temp;
		i++;
	} while((temp & 128) != 0 && i < 16);

	unsigned int len = circles_uleb128_decode(i, bytes);
	*ptr = (char*) malloc(len + 1);
	if(*ptr == NULL)
		return CIRCLES_ERROR_ALLOC_FAILED;

	temp_size = len;
	if((*callback)(*ptr, &temp_size, ctx) || temp_size != len)
		return CIRCLES_ERROR_BROKEN_STREAM;

	(*ptr)[len] = 0;

	return 0;
}
