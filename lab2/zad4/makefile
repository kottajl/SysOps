SHELL= /bin/bash

all:
	g++ -Wall main.c -o main


test:
	make clean
	make all
	./main .
	# find . | xargs wc --bytes
	make clean


clean:
	rm -f *.o
	rm -f main
