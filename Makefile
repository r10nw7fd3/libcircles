OUT=libcircles.so
CFLAGS=-Wall -Wextra -std=c99 -O2 -fpic
SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)

all: $(OUT) include

$(OUT): $(OBJ)
	$(CC) -shared -o $(OUT) $(OBJ) -leasylzma_s

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

include: src/replay.h src/util.h
	mkdir -p include/libcircles
	cp -t include/libcircles src/replay.h src/util.h

clean:
	rm -f $(OUT) $(OBJ)
	rm -rf include

.PHONY: all clean include
