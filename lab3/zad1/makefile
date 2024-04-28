SHELL= /bin/bash

all:
	gcc -Wall main.c -o main


test:
	make clean
	make all
	echo "Test for seven child processes:"
	./main 7
	make clean


clean:
	rm -f main