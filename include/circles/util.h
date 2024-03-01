#ifndef _LIBCIRCLES_UTIL_H_
#define _LIBCIRCLES_UTIL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

typedef int (*circles_read_fn_t)(char* buf, size_t* size, void* ctx);

enum {
	CIRCLES_MODE_STANDART = 0,
	CIRCLES_MODE_TAIKO = 1,
	CIRCLES_MODE_CATCH = 2,
	CIRCLES_MODE_MANIA = 3
};

typedef enum {
	CIRCLES_ERROR_OK = 0,
	CIRCLES_ERROR_OPEN_FAILED = 1,
	//CIRCLES_ERROR_EOF = 2,
	CIRCLES_ERROR_ALLOC_FAILED = 3,
	CIRCLES_ERROR_FILE_CORRUPTED = 4,
	CIRCLES_ERROR_LZMA = 5,
	CIRCLES_ERROR_BROKEN_STREAM = 6,
	CIRCLES_ERROR_UNKNOWN = 7,
	CIRCLES_ERROR_INVALID_ARGUMENT = 8
} CirclesError;

uint64_t circles_uleb128_decode(int size, const char* bytes);
CirclesError circles_binstring_decode(char** dest, circles_read_fn_t callback, void* ctx);

static inline int64_t circles_jesustime_to_unix(int64_t ticks) {
	return (ticks - 621355968000000000 ) / 10000000;
}

#ifdef __cplusplus
}
#endif

#endif
