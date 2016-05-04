#!/usr/bin/make

CFLAGS=-g

all: forkpty.so

forkpty.so: forkpty.c
	$(CC) $(CFLAGS) -DFORKPTY -shared -fPIC -o $@ $^

forkpty-test: forkpty.c
	$(CC) $(CFLAGS) -DFORKPTY -DMAIN -o $@ $^

clean:
	rm -f forkpty.so forkpty-test
