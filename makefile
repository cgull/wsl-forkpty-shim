#!/usr/bin/make

CFLAGS=-g

forkpty.so: forkpty.c
	$(CC) $(CFLAGS) -DFORKPTY -shared -fPIC -o $@ $^

forkpty-test: forkpty.c
	$(CC) $(CFLAGS) -DFORKPTY -DMAIN -o $@ $^

openpty-test: forkpty.c
	$(CC) $(CFLAGS) -DMAIN -o $@ $^ -lutil

all: forkpty.so forkpty-test openpty-test

clean:
	rm -f forkpty.so forkpty-test openpty-test
