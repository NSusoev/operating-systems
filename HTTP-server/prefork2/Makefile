CC = gcc

preforkserver: server.o answer.o 
	$(CC) -o preforkserver server.o answer.o

server.o: server.c  headers/answer.h
	$(CC) -c server.c 

answer.o: answer.c  headers/answer.h
	$(CC) -c answer.c
