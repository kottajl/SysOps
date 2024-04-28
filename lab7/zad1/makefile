SHELL= /bin/bash

all:
	gcc -Wall main.c -o main
	gcc -Wall barber.c -o barber
	gcc -Wall client.c -o client


help:
	echo "Usage: 'make -s run ARGS=\"<M> <N> <P> <F>\"'"
	echo "You can also type 'make -s test'"


run:
	make all
	./main $(ARGS)


test:
	echo "Test for 2 barbers, 1 armchair, 3 chairs in waiting room and 20 haircut styles"
	echo " "
	make clean
	make run ARGS="2 1 3 20"


clean:
	rm -f main
	rm -f client
	rm -f barber