CC = gcc
FLAGS = `pkg-config fuse --cflags --libs`

filesystem: filesystem.c fsoperations.c struct.h fsoperations.h
	$(CC) -Wall filesystem.c fsoperations.c $(FLAGS) -o filesystem

