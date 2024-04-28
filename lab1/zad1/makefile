CC= gcc -Wall

static:
	$(CC) -c lib.c
	ar rcs lib.a lib.o

shared:
	$(CC) -fPIC -c lib.c
	$(CC) -shared lib.o -o liblib.so


	