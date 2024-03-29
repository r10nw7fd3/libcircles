#include <circles/replay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lzma.h"

static int cleanup(int code, Replay** replay) {
	circles_replay_end(replay);
	return code;
}

// TODO: Reduce code duplication
static int hpbar_parse(HPSequence** hp, char* string, size_t* hp_num) {
	if(!string)
		return 0;

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

CirclesError circles_replay_parse(Replay** replay_dest, circles_read_fn_t callback, void* ctx) {
	if(!replay_dest || !callback)
		return CIRCLES_ERROR_INVALID_ARGUMENT;

	*replay_dest = (Replay*) malloc(sizeof(Replay));
	if(!*replay_dest)
		return CIRCLES_ERROR_ALLOC_FAILED;

	Replay* replay = *replay_dest;

	replay->map_md5[0] = 0;
	replay->replay_md5[0] = 0;
	replay->player = NULL;
	replay->hp = NULL;
	replay->hp_num = 0;
	replay->frames = NULL;
	replay->frames_num = 0;
	char tmp;

#define CALLCHECK(dest, size, err) \
	{ \
		size_t size_var = size; \
		if((*callback)((char*) dest, &size_var, ctx) || size_var != size) { \
			circles_replay_end(replay_dest); \
			return err; \
		} \
	}

#define CALLCHECK_BSTREAM(dest, size) CALLCHECK(dest, size, CIRCLES_ERROR_BROKEN_STREAM)

	CALLCHECK_BSTREAM(&replay->mode, 1)
	CALLCHECK_BSTREAM(&replay->version, 4)

	CALLCHECK_BSTREAM(&tmp, 1)
	if(tmp == 11) {
		CALLCHECK_BSTREAM(&tmp, 1)
		if(tmp != 32)
			return CIRCLES_ERROR_FILE_CORRUPTED;
		CALLCHECK_BSTREAM(replay->map_md5, 32)
		replay->map_md5[32] = 0;
	}

	int exitcode = circles_binstring_decode(&replay->player, callback, ctx);
	if(exitcode)
		return cleanup(exitcode, replay_dest);

	CALLCHECK_BSTREAM(&tmp, 1)
	if(tmp == 11) {
		CALLCHECK_BSTREAM(&tmp, 1)
		if(tmp != 32)
			return CIRCLES_ERROR_FILE_CORRUPTED;
		CALLCHECK_BSTREAM(replay->replay_md5, 32)
		replay->replay_md5[32] = 0;
	}

	CALLCHECK_BSTREAM(&replay->hit300, 2)
	CALLCHECK_BSTREAM(&replay->hit100, 2)
	CALLCHECK_BSTREAM(&replay->hit50, 2)
	CALLCHECK_BSTREAM(&replay->geki, 2)
	CALLCHECK_BSTREAM(&replay->katu, 2)
	CALLCHECK_BSTREAM(&replay->miss, 2)
	CALLCHECK_BSTREAM(&replay->score, 4)
	CALLCHECK_BSTREAM(&replay->combo, 2)
	CALLCHECK_BSTREAM(&replay->perfect, 1)
	// Not really sure how we should interface this
	CALLCHECK_BSTREAM(&replay->mods, 4)

	char* hp;
	exitcode = circles_binstring_decode(&hp, callback, ctx);
	if(exitcode) {
		free(hp);
		return cleanup(exitcode, replay_dest);
	}

	exitcode = hpbar_parse(&replay->hp, hp, &replay->hp_num);
	free(hp);
	if(exitcode)
		return cleanup(exitcode, replay_dest);

	CALLCHECK_BSTREAM(&replay->time, 8)
	replay->time = circles_jesustime_to_unix(replay->time);

	size_t lzma_size;
	CALLCHECK_BSTREAM(&lzma_size, 4)

	if(!lzma_size)
		return 0; // Nothing to decompress and parse

	char* lzma = (char*) malloc(lzma_size);
	if(lzma == NULL)
		return cleanup(CIRCLES_ERROR_ALLOC_FAILED, replay_dest);
	CALLCHECK_BSTREAM(lzma, lzma_size)

	DataStream ds;
	ds.in_size = lzma_size;
	ds.in = lzma;
	ds.out = NULL;

	int ret = circles_lzma_decompress(&ds);
	free(lzma);
	if(ret) {
		free(ds.out);
		return cleanup(ret, replay_dest);
	}
	
	ds.out = realloc(ds.out, ds.out_size + 1);
	ds.out[ds.out_size] = 0;

	exitcode = frames_parse(&replay->frames, (char*) ds.out, &replay->frames_num);
	free(ds.out);
	if(exitcode)
		return cleanup(exitcode, replay_dest);

	return 0;
}

void circles_replay_end(Replay** replay_dest) {
	if(!replay_dest)
		return;

	Replay* replay = *replay_dest;

	free(replay->player);
	free(replay->hp);
	free(replay->frames);

	free(replay);
	*replay_dest = NULL;
}

static int read_callback(char* buf, size_t* size, void* ctx) {
	FILE* fp = (FILE*) ctx;

	if(fread(buf, *size, 1, fp) != 1)
		return CIRCLES_ERROR_BROKEN_STREAM;

	return 0;
}

CirclesError circles_replay_parse_file(Replay** replay, const char* fname) {
	FILE* fp = fopen(fname, "rb");
	if(fp == NULL)
		return CIRCLES_ERROR_OPEN_FAILED;

	int res = circles_replay_parse(replay, &read_callback, (void*) fp);
	fclose(fp);

	return res;
}
