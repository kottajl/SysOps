CC= gcc -Wall
renameO= mv results_static.txt results_static_O.txt; mv results_shared.txt results_shared_O.txt; mv results_dynamic.txt results_dynamic_O.txt;
renameO0= mv results_static.txt results_static_O0.txt; mv results_shared.txt results_shared_O0.txt; mv results_dynamic.txt results_dynamic_O0.txt; 
renameO1= mv results_static.txt results_static_O1.txt; mv results_shared.txt results_shared_O1.txt; mv results_dynamic.txt results_dynamic_O1.txt; 
renameO2= mv results_static.txt results_static_O2.txt; mv results_shared.txt results_shared_O2.txt; mv results_dynamic.txt results_dynamic_O2.txt; 
renameO3= mv results_static.txt results_static_O3.txt; mv results_shared.txt results_shared_O3.txt; mv results_dynamic.txt results_dynamic_O3.txt;
renameOs= mv results_static.txt results_static_Os.txt; mv results_shared.txt results_shared_Os.txt; mv results_dynamic.txt results_dynamic_Os.txt;

all:
	make static
	make shared
	make dynamic

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
	

single_test:
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

	./static_main < /tmp/input_commands > results_static.txt
	./shared_main < /tmp/input_commands > results_shared.txt
	./dynamic_main < /tmp/input_commands > results_dynamic.txt

	rm -f /tmp/input_commands
	make clean


test:
	make single_test O=-O
	$(renameO)

	make single_test O=-O0
	$(renameO0)

	make single_test O=-O1
	$(renameO1)

	make single_test O=-O2
	$(renameO2)

	make single_test O=-O3
	$(renameO3)

	make single_test O=-Os
	$(renameOs)


clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f *.so.*