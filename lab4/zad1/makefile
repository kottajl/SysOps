SHELL= /bin/bash

all:
	gcc -Wall fork.c -o fork
	gcc -Wall exec.c -o exec
	gcc -Wall exec_child.c -o exec_child


test:
	make clean
	make all

	echo "Testing 'fork':"
	echo "- ignore"
	./fork ignore
	echo "- handler"
	./fork handler
	echo "- mask"
	./fork mask
	echo "- pending"
	./fork pending
	echo ""

	echo "Testing 'exec':"
	echo "- ignore"
	./exec ignore
	echo "- mask"
	./exec mask
	echo "- pending"
	./exec pending

	make clean	

clean:
	rm -f fork
	rm -f exec
	rm -f exec_child