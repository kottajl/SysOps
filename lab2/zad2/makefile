SHELL= /bin/bash

all:
	g++ -Wall mainSingle.c -o single
	g++ -Wall mainBlocks.c -o blocks


test:
	make clean
	make all

	./single text1.txt tmp
	./single tmp out
	diff -s text1.txt out

	./blocks text1.txt tmp
	./blocks tmp out
	diff -s text1.txt out
	make clean
	

clean:
	rm -f tmp
	rm -f out
	rm -f single
	rm -f blocks
	rm -f *.o