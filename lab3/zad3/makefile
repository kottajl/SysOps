SHELL= /bin/bash

all:
	gcc -Wall main.c -o main


test:
	make clean
	make all

	echo "Test1 - files in . that begin with '#inc':"
	./main . \#inc

	echo ""
	echo "Test2 - files in WaclawikAndrzej that begin with '#include':"
	./main ../.. \#include

	echo ""
	echo "Test3 - files in ~ that are empty (begin with '\0'):"
	./main ~ \0

	echo ""
	echo "Test4 - files in ~ that begin with 'SH':"
	./main ~ SH

	echo ""
	echo "Test5 - files in ~ that begin with 'asdf':"
	./main ~ asdf

	make clean


clean:
	rm -f main