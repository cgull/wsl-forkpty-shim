#!/usr/bin/make

CFLAGS=-g

all: openpty.so

openpty.so: openpty.c
	$(CC) $(CFLAGS) -DFORKPTY -shared -fPIC -g -o $@ $^
