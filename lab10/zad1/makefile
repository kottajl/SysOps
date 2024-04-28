SHELL= /bin/bash

all:
	gcc -Wall server.c -o server -lpthread
	gcc -Wall client.c -o client


server-go:
	./server 9009 127.0.0.1


client-go:
	./client my_nick 127.0.0.1 9009
	

clean:
	rm -f server
	rm -f client