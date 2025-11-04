CC = gcc
CFLAGS = -Wall -g -pedantic

all: sched

sched: sched.o
	$(CC) $(CFLAGS) sched.o -o sched

sched.o: sched.c
	$(CC) $(CFLAGS) -c sched.c

clean:
	rm -f sched sched.o
