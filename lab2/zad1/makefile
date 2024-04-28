SHELL= /bin/bash

all:
	g++ -Wall mainLib.c -o libfar
	g++ -Wall mainSys.c -o sysfar

test:
	make clean
	make all
	tr v q < text1.txt > tmp

	./libfar v q text1.txt out
	diff -s out tmp

	./sysfar v q text1.txt out
	diff -s out tmp
	make clean


clean:
	rm -f tmp
	rm -f out
	rm -f libfar
	rm -f sysfar
	rm -f *.o