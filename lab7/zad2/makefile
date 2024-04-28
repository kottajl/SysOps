SHELL= /bin/bash

all:
	gcc -Wall main.c -o main -lpthread -lrt
	gcc -Wall barber.c -o barber -lpthread -lrt
	gcc -Wall client.c -o client -lpthread -lrt


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