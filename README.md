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

### Includes

```c
#include <libcircles/replay.h>
#include <libcircles/util.h>
```

### Reading from file

```c
Replay replay;
circles_replay_fromfile(&replay, "test.osr"); // Returns 0 on success
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
circles_replay_parse(&replay, &read_callback, (void*) fp); // Returns 0 on success
fclose(fp);
// See src/replay.h for available attributes
```

### Dealloc

```c
circles_replay_end(&replay);
```
