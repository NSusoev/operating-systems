CC = gcc

CFLAGS = -lpthread

forkserver: fork1http.o
	$(CC) $(CFLAGS) -o forkserver fork1http.c

fork1http.o: fork1http.c
	$(CC) $(CFLAGS) -c fork1http.c
