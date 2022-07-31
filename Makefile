# Execution command- "./all <input> output"
CC=gcc

option=-c

all:    main.o babystep3.o myalloc.o
	$(CC) main.o babystep3.o myalloc.o -o all

babystep3.o: babystep3.c driver.h
	$(CC) $(option) babystep3.c

myalloc.o: myalloc.c driver.h
	$(CC) $(option) myalloc.c

main.o: main.c driver.h
	$(CC) $(option) main.c

clean:
	rm -rf *o all

