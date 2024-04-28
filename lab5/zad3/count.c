#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <math.h>

double f (double x) {
    return 4 / (x * x + 1);
}


int main (int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "Wrong number of arguments! - %d", argc);
        return -1;
    }

    double a= atof(argv[1]);
    double b= atof(argv[2]);
    long long n= atoll(argv[3]);
    int fifo= atoi(argv[4]);

    double dx= (b - a) / n;
    double sum= 0.0;

    for (int i=0; i<n; i++)
        sum+= f(a + dx * i);
    sum*= dx;

    if (fifo == -1) {
        fprintf(stderr, "a=%lf: ", a);
        perror("Error in opening fifo (write mode)");
        return -1;
    }
    if (write(fifo, &sum, sizeof(double)) == -1) {
        fprintf(stderr, "%lf - %lf: ", a, b);
        perror("Error in writing to fifo");
        return -1;
    }
    
    close(fifo);

    return 0;
}