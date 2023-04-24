#ifndef _LIBCIRCLES_LZMA_H_
#define _LIBCIRCLES_LZMA_H_

#include <stdlib.h>

// Stolen from https://github.com/lloyd/easylzma/blob/master/test/simple.c#L18
typedef struct {
	size_t in_size;
	const char* in;
	size_t out_size;
	char* out;
} DataStream;

int _lzma_decompress(DataStream*);

#endif
