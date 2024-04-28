SHELL= /bin/bash

all:
	gcc -Wall main.c -o main


test:
	make clean
	make all
	
	echo "dx = 0.0000000015, n = 15" > raport.txt
	./main 0.0000000015 15 >> raport.txt
	echo "" >> raport.txt

	echo "dx = 0.000000001, n = 5" >> raport.txt
	./main 0.000000001 5 >> raport.txt
	echo "" >> raport.txt

	echo "dx = 0.000000005, n = 1" >> raport.txt
	./main 0.000000005 1 >> raport.txt
	echo "" >> raport.txt

	echo "dx = 0.000000005, n = 4" >> raport.txt
	./main 0.000000005 4 >> raport.txt
	echo "" >> raport.txt

	echo "dx = 0.000000005, n = 8" >> raport.txt
	./main 0.000000005 8 >> raport.txt
	echo "" >> raport.txt
	
	echo "dx = 0.000000001, n = 8" >> raport.txt
	./main 0.000000001 8 >> raport.txt

	make clean


clean:
	rm -f main