# libcircles

osu! Replay parser

This library implements [easylzma](https://github.com/lloyd/easylzma)-like api,
which allows the user to parse replays directly from memory, without having to save them to disk.

Easylzma is also a dependency, needed for replay decompression

## Building

Install easylzma

```
make
```

## Usage

### Includes

```c
#include <libcircles/replay.h>
#include <libcircles/util.h>
```

### Reading from file

```c
Replay replay;
int ret = circles_replay_fromfile(&replay, "test.osr"); // 0 == success
// See src/replay.h for available attributes
```

### Reading from wherever

#### Callback

```c
int read_callback(void* ctx, char* buf, size_t size) {
	FILE* fp = (FILE*) ctx;

	if(fread(buf, size, 1, fp) != 1)
		return 1;

	return 0;
}
```

#### Parsing

```c
FILE* fp = fopen("test.osr", "rb");
Replay replay;
int result = circles_replay_parse(&replay, &read_callback, (void*) fp);
fclose(fp);
// See src/replay.h for available attributes
```

### Dealloc

```
circles_replay_end(&replay);
```
