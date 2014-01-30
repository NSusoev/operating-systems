CC = gcc
FLAGS = -lfuse

fstest: fsactions.o fat_test.o
	$(CC) -o fstest fsactions.c fat_test.o $(FLAGS)

fsactions.o: fsactions.c fsactions.h
	$(CC) -c fsactions.c

fat_test.o: fat_test.c fsactions.h
	$(CC) -c fat_test.c
