SHELL= /bin/bash

all:
	make clean
	make comp
	./main


comp:
	g++ -Wall main.c -o main


test:
	make all
	wc --bytes *
	make clean


clean:
	rm -f *.o
	rm -f main
