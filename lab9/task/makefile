SHELL= /bin/bash

all:
	gcc -Wall main.c -o main -lpthread


test:
	make clean
	make all
	./main
	make clean


clean:
	rm -f main