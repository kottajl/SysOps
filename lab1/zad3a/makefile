CC= gcc -Wall

static:
	$(CC) -c lib.c
	ar rcs lib.a lib.o
	$(CC) -c main.c
	$(CC)  main.o lib.o -o static_main

shared:
	$(CC) -fPIC -c lib.c
	$(CC) -shared lib.o -o liblib.so
	$(CC) -c main.c
	$(CC) main.o -llib -W1 -L. -o shared_main

dynamic:
	$(CC) -fPIC -c lib.c
	$(CC) -shared lib.o -o liblib.so
	$(CC) main2.c -o dynamic_main

all:
	make static
	make shared
	make dynamic

test:
	make all

	touch /tmp/input_commands
	echo "init 5" >> /tmp/input_commands

	echo "count lib.h" >> /tmp/input_commands
	echo "count lib.c" >> /tmp/input_commands
	echo "count main.c" >> /tmp/input_commands
	echo "count main2.c" >> /tmp/input_commands

	echo "show 0" >> /tmp/input_commands
	echo "show 1" >> /tmp/input_commands
	echo "show 2" >> /tmp/input_commands
	echo "show 3" >> /tmp/input_commands

	echo "delete 0" >> /tmp/input_commands
	echo "delete 0" >> /tmp/input_commands
	echo "delete 0" >> /tmp/input_commands
	echo "delete 0" >> /tmp/input_commands

	echo "destroy" >> /tmp/input_commands

	./static_main < /tmp/input_commands > ./results_static.txt
	./shared_main < /tmp/input_commands > ./results_shared.txt
	./dynamic_main < /tmp/input_commands > ./results_dynamic.txt

	rm -f /tmp/input_commands
	make clean

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f *.so.*