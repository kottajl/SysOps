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

clean:
	rm -f *.o
	rm -f *.a
	rm -f *.so
	rm -f *.so.*