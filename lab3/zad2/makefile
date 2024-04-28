SHELL= /bin/bash

all:
	gcc -Wall main.c -o main


test:
	make clean
	make all

	echo ""
	echo "-> Directory with zero files:"
	mkdir empty_dir
	./main empty_dir

	echo ""
	echo "-> Directory with several files (.):"
	./main .

	echo "-> Directory with many files (~):"
	./main ~

	make clean


clean:
	rm -f main
	rm -fr empty_dir