#include "lzma.h"
#include <string.h>
#include <stdlib.h>
#include <easylzma/decompress.h>
#include <libcircles/util.h>

// Stolen from https://github.com/lloyd/easylzma/blob/master/test/simple.c#L18
static int callback_in(void* ctx, void* buf, size_t* size) {
	DataStream* ds = (DataStream*) ctx;

	size_t rd = 0;
	rd = (ds->in_size < *size) ? ds->in_size : *size;

	if(rd > 0) {
		memcpy(buf, (void*) ds->in, rd);
		ds->in += rd;
		ds->in_size -= rd;
	}
	*size = rd;

	return 0;
}

static size_t callback_out(void* ctx, const void* buf, size_t size) {
	DataStream* ds = (DataStream*) ctx;

	if(size > 0) {
		ds->out = realloc(ds->out, ds->out_size + size);
		memcpy((void*) (ds->out + ds->out_size), buf, size);
		ds->out_size += size;
	}

	return size;
}

int circles_lzma_decompress(DataStream* ds) {
	if(ds == NULL)
		return 1;

	ds->out_size = 0;
	ds->out = NULL;

	elzma_decompress_handle handle = NULL;
	handle = elzma_decompress_alloc();
	if(handle == NULL)
		return CIRCLES_ERROR_ALLOC_FAILED;

	int ret = elzma_decompress_run(handle, callback_in, (void*) ds, callback_out, (void*) ds, ELZMA_lzma);
	elzma_decompress_free(&handle);

	if(ret != ELZMA_E_OK)
		return CIRCLES_ERROR_LZMA;

	return 0;
}
