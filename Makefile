OUT=libcircles.so
CFLAGS=-Wall -Wextra -std=c99 -O2 -fPIC -Iinclude
PREFIX?=/usr/local

SRC=$(wildcard src/*.c)
OBJ=$(SRC:.c=.o)

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) -shared -o $(OUT) $(OBJ) -leasylzma_s

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OUT) $(OBJ)

install: all
	cp $(OUT) $(PREFIX)/lib
	cp -r include/libcircles $(PREFIX)/include

uninstall:
	rm -f $(PREFIX)/lib/$(OUT)
	rm -rf $(PREFIX)/include/libcircles

.PHONY: all clean install uninstall
