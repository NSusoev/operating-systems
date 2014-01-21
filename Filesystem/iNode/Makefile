CC = gcc
FLAGS = -lfuse

fs: filesystem.o fsactions.o
	$(CC) -o fs filesystem.o fsactions.o $(FLAGS)

filesystem.o: filesystem.c fsactions.h
	$(CC) -c filesystem.c

fsactions.o: fsactions.c fsactions.h
	$(CC) -c fsactions.c
