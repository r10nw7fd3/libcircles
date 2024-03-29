#ifndef _LIBCIRCLES_REPLAY_H_
#define _LIBCIRCLES_REPLAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "util.h"

typedef struct {
	uint32_t ms;
	float hp;
} HPSequence;

typedef struct {
	int64_t prev;
	float x;
	float y;
	int32_t keys;
} ReplayFrame;

typedef struct {
	char mode;
	int32_t version;
	char map_md5[33];
	char* player;
	char replay_md5[33];
	int16_t hit300;
	int16_t hit100;
	int16_t hit50;
	int16_t geki;
	int16_t katu;
	int16_t miss;
	int32_t score;
	int16_t combo;
	uint8_t perfect;
	int32_t mods;
	size_t hp_num; // Number of HPSequence elements
	HPSequence* hp;
	int64_t time; // Time in seconds since unix epoch
	size_t frames_num; // Number of ReplayFrame elements
	ReplayFrame* frames;
	int32_t id;
	int64_t mod_info;
} Replay;

CirclesError circles_replay_parse(Replay** replay, circles_read_fn_t callback, void* ctx);
CirclesError circles_replay_parse_file(Replay** replay, const char* filename);
void circles_replay_end(Replay** replay);

#ifdef __cplusplus
}
#endif

#endif
