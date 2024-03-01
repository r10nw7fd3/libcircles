# libcircles

osu! Replay parser

This library implements callback api,
which allows the user to parse replays directly from memory, without having to save them to disk.

[Easylzma](https://github.com/lloyd/easylzma) is the only dependency, needed for replay decompression

## Building

Install easylzma and run

```
make
```

## Usage

### Include

```c
#include <libcircles/replay.h>
```

### Reading from file

```c
Replay* replay;
circles_replay_parse_file(&replay, "test.osr"); // Returns 0 on success
// See include/circles/replay.h for available attributes
circles_replay_end(&replay);
```

### Reading from wherever

#### Callback

```c
int read_callback(char* buf, size_t* size, void* ctx) {
	FILE* fp = (FILE*) ctx;

	return fread(buf, *size, 1, fp) != 1;
}
```

#### Parsing

```c
FILE* fp = fopen("test.osr", "rb");
Replay* replay;
circles_replay_parse(&replay, &read_callback, (void*) fp); // Returns 0 on success
fclose(fp);
// See include/circles/replay.h for available attributes
circles_replay_end(&replay);
```
