OUT=libcircles.so
CFLAGS=-Wall -Wextra -std=c99 -O2 -fpic
PREFIX?=/usr/local
SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)

all: $(OUT) include

$(OUT): $(OBJ)
	$(CC) -shared -o $(OUT) $(OBJ) -leasylzma_s

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OUT) $(OBJ)
	rm -rf include

include: src/replay.h src/util.h
	mkdir -p include/libcircles
	cp -t include/libcircles src/replay.h src/util.h

install: all
	cp $(OUT) $(PREFIX)/lib
	cp -r include/libcircles $(PREFIX)/include

uninstall:
	rm -f $(PREFIX)/lib/$(OUT)
	rm -rf $(PREFIX)/include/libcircles

.PHONY: all clean include install uninstall
