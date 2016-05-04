#!/usr/bin/make

CFLAGS=-g

all: forkpty.so

forkpty.so: forkpty.c
	$(CC) $(CFLAGS) -DFORKPTY -shared -fPIC -g -o $@ $^
