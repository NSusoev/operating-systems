CC = gcc

INCLUDE =./headers

preforkserver: server.o answer.o 
	$(CC) -o preforkserver server.o answer.o

server.o: server.c answer.h 
	$(CC) -I$(INCLUDE) -c server.c

answer.o: answer.c answer.h
	$(CC) -I$(INCLUDE) -c answer.c

