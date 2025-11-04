CC = gcc
CFLAGS = -Wall -g -pedantic

all: mymalloc

mymalloc: mymalloc.o
	$(CC) $(CFLAGS) mymalloc.o -o mymalloc

mymalloc.o: mymalloc.c
	$(CC) $(CFLAGS) -c mymalloc.c

clean:
	rm -f mymalloc mymalloc.o
