#ifndef _LIBCIRCLES_UTIL_H_
#define _LIBCIRCLES_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

typedef int (*CirclesCallbackRead)(void* ctx, char* buf, size_t* size);

enum {
	CIRCLES_MODE_STANDART,
	CIRCLES_MODE_TAIKO,
	CIRCLES_MODE_MANIA,
	CIRCLES_MODE_CATCH
};

enum {
	CIRCLES_ERROR_OPEN_FAILED = 1,
	//CIRCLES_ERROR_EOF = 2,
	CIRCLES_ERROR_ALLOC_FAILED = 3,
	CIRCLES_ERROR_FILE_CORRUPTED = 4,
	CIRCLES_ERROR_LZMA = 5,
	CIRCLES_ERROR_BROKEN_STREAM = 6,
	CIRCLES_ERROR_UNKNOWN = 7,
};

unsigned int circles_uleb128_decode(int size, char* bytes);
int circles_fpstring_parse(char** dest, CirclesCallbackRead callback, void* ctx);
long long circles_jesustime_to_unixms(long long time_in_ticks);

#ifdef __cplusplus
}
#endif

#endif
