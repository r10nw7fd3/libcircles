#include "replay.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lzma.h"

static int cleanup(int code, Replay* replay) {
	circles_replay_end(replay);
	return code;
}

// TODO: Reduce code duplication
static int hpbar_parse(HPSequence** hp, char* string, size_t* hp_num) {
	size_t len = strlen(string);

	int alloced_elements = 0;
	float multiplier = 10;
	int status = 0; // 0 - time, 1 - value
	int result_time = 0;
	float result_value = 0.0f;

	for(size_t i = 0; i < len; i++) {
		if(string[i] >= '0' && string[i] <= '9') {
			if(status == 0) {
				result_time *= 10;
				result_time += string[i] - '0';
			}
			else {
				result_value *= multiplier;
				result_value += string[i] - '0';
			}

		}
		else if(string[i] == '|')
			status = !status;
		else if(string[i] == ',') {
			// (Re)Allocate memory for a new sequence
			// Write result_time and result_value
			// Reset result_time and result value;
			// Switch status

			if(result_value != 1)
				result_value /= 10;

			*hp = (HPSequence*) realloc(*hp, sizeof(HPSequence) * ++alloced_elements);
			if(*hp == NULL)
				return CIRCLES_ERROR_ALLOC_FAILED;

			(*hp)[alloced_elements - 1].ms = result_time;
			(*hp)[alloced_elements - 1].hp = result_value;

			multiplier = 10;
			result_time = 0;
			result_value = 0.0f;
			status = 0;
		}
		else if(string[i] == '.')
			multiplier = 0.1f;
		else
			return CIRCLES_ERROR_FILE_CORRUPTED;
	}
	*hp_num = alloced_elements;

	return 0;
}

// TODO: Rewrite this completely
static int frames_parse(ReplayFrame** frames, char* string, size_t* frame_num)  {
	size_t len = strlen(string);

	const int buf_size = 16;
	char buf[buf_size];
	memset(buf, 0, buf_size); // Unnecessary, but just in case
	int buf_pos = 0;

	int alloced_elements = 0;
	int status = 0; // 0 - dtime, 1 - x, 2 - y, 3 - keys

	long long result_dtime = 0;
	float result_x = 0.0f;
	float result_y = 0.0f;
	int result_keys = 0;

	int sign_x = 1;
	int sign_y = 1;
	int sign_dtime = 1;

	for(size_t i = 0; i < len; i++) {
		if(string[i] >= '0' && string[i] <= '9') {
			if(status == 0) {
				result_dtime *= 10;
				result_dtime += string[i] - '0';
			}
			else if(status == 1 || status == 2) {
				buf[buf_pos++] = string[i];
			}
			else {
				result_keys *= 10;
				result_keys += string[i] - '0';
			}
		}
		else if(string[i] == '|') {
			if(status == 1) {
				result_x = strtof(buf, NULL);
				result_x *= sign_x;
			}
			if(status == 2) {
				result_y = strtof(buf, NULL);
				result_y *= sign_y;
			}
			memset(buf, 0, buf_size);
			buf_pos = 0;
			status++;
		}
		else if(string[i] == ',') {
			result_dtime *= sign_dtime;

			*frames = (ReplayFrame*) realloc(*frames, sizeof(ReplayFrame) * ++alloced_elements);
			if(*frames == NULL)
				return CIRCLES_ERROR_ALLOC_FAILED;

			(*frames)[alloced_elements - 1].prev = result_dtime;
			(*frames)[alloced_elements - 1].x = result_x;
			(*frames)[alloced_elements - 1].y = result_y;
			(*frames)[alloced_elements - 1].keys = result_keys;

			result_dtime = 0;
			result_x = 0.0f;
			result_y = 0.0f;
			result_keys = 0;
			status = 0;
			sign_dtime = 1;
			sign_x = 1;
			sign_y = 1;
		}
		else if(string[i] == '.') {
			buf[buf_pos++] = '.';
		}
		else if(string[i] == '-') // This is just dumb
			if(status == 0)
				sign_dtime = -1; // This should not be possible, but some replays actually contain negative delta time
			else if(status == 1) // osu! wiki says that x is a value between 0 and 512, but seems that it is wrong
				sign_x = -1;
			else if(status == 2) // Should be between 0 and 384
				sign_y = -1;
			else
				return CIRCLES_ERROR_UNKNOWN;

		else
			return CIRCLES_ERROR_FILE_CORRUPTED;
	}

	*frame_num = alloced_elements;
	return 0;
}

int circles_replay_parse(Replay* replay, CirclesCallbackRead callback, void* ctx) {
	replay->player = NULL;
	replay->hp = NULL;
	replay->frames = NULL;
	char devnull[512];


	if((*callback)(ctx, (char*) &replay->mode, 1))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	if((*callback)(ctx, (char*) &replay->version, 4))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

    // Skip identifier and uleb since we already know the length
	if((*callback)(ctx, devnull, 2))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	if((*callback)(ctx, replay->map_md5, 32))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	replay->map_md5[32] = 0;

	// And now the fun part

	int exitcode = circles_fpstring_parse(&replay->player, callback, ctx);
	if(exitcode)
		return cleanup(exitcode, replay);

	// Same
	(*callback)(ctx, devnull, 2);

	if((*callback)(ctx, replay->replay_md5, 32))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	replay->replay_md5[32] = 0;

	if((*callback)(ctx, (char*) &replay->hit300, 2))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	if((*callback)(ctx, (char*) &replay->hit100, 2))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	if((*callback)(ctx, (char*) &replay->hit50, 2))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	if((*callback)(ctx, (char*) &replay->geki, 2))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	if((*callback)(ctx, (char*) &replay->katu, 2))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	if((*callback)(ctx, (char*) &replay->miss, 2))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	if((*callback)(ctx, (char*) &replay->score, 4))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	if((*callback)(ctx, (char*) &replay->combo, 2))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	if((*callback)(ctx, (char*) &replay->perfect, 1))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	// Not really sure how we should interface this
	if((*callback)(ctx, (char*) &replay->mods, 4))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	char* hp;
	exitcode = circles_fpstring_parse(&hp, callback, ctx);
	if(exitcode)
		return cleanup(exitcode, replay);

	exitcode = hpbar_parse(&replay->hp, hp, &replay->hp_num);
	if(exitcode)
		return cleanup(exitcode, replay);

	if((*callback)(ctx, (char*) &replay->time, 8))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	size_t lzma_size;
	if((*callback)(ctx, (char*) &lzma_size, 4))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	char* lzma = (char*) malloc(lzma_size);
	if((*callback)(ctx, lzma, lzma_size))
		return cleanup(CIRCLES_ERROR_BROKEN_STREAM, replay);

	DataStream ds;
	ds.in_size = lzma_size;
	ds.in = lzma;

	int ret = _lzma_decompress(&ds);
	if(ret)
		return cleanup(ret, replay);
	free(lzma);
	
	ds.out = realloc(ds.out, ds.out_size + 1);
	ds.out[ds.out_size] = 0;

	exitcode = frames_parse(&replay->frames, (char*) ds.out, &replay->frame_num);
	if(exitcode)
		return cleanup(exitcode, replay);

	return 0;
}

void circles_replay_end(Replay* replay) {
	if(replay->player != NULL) {
		free(replay->player);
		replay->player = NULL;
	}

	if(replay->hp != NULL) {
		free(replay->hp);
		replay->hp = NULL;
	}

	if(replay->frames != NULL) {
		free(replay->frames);
		replay->frames = NULL;
	}
}

static int _read_callback(void* ctx, char* buf, size_t size) {
	FILE* fp = (FILE*) ctx;

	if(fread(buf, size, 1, fp) != 1)
		return CIRCLES_ERROR_BROKEN_STREAM;

	return 0;
}

int circles_replay_fromfile(Replay* replay, char* fname) {
	FILE* fp = fopen(fname, "rb");
	if(fp == NULL)
		return CIRCLES_ERROR_OPEN_FAILED;

	int res = circles_replay_parse(replay, &_read_callback, (void*) fp);
	fclose(fp);

	return res;
}
