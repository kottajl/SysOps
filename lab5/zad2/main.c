#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>


double f (double x) {
    return 4 / (x * x + 1);
}


int main (int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Wrong number of arguments!\n");
        return -1;
    }

    struct timeval start, end;

    double dx= atof(argv[1]);
    int n= atoi(argv[2]);

    int fd[n][2];
    for (int i=0; i<n; i++)
        if (pipe(fd[i])) {
            fprintf(stderr, "Plumbing error!\n");
            return -1;
        }

    gettimeofday(&start, NULL);

    long long total_tasks= 1.0 / dx;
    long long tasks_per_wzium= total_tasks / n;
    long long tasks[n];
    for (int i=0; i<n; i++)
        tasks[i]= tasks_per_wzium;

    int temp= total_tasks % n;
    for (int i=0; i < temp; i++)
        tasks[i]++;

    long long tasks_sum[n];
    tasks_sum[0]= 0;
    for (int i=1; i<n; i++)
        tasks_sum[i]= tasks_sum[i-1] + tasks[i-1];

    for (int i=0; i<n; i++) {
        int pid= fork();

        if (pid == 0) {
            close(fd[i][0]);

            double sum= 0;
            for (long long j=0; j<tasks[i]; j++) {
                double x= dx * (tasks_sum[i] + j);
                sum+= f(x);
            }

            sum*= dx;
            write(fd[i][1], &sum, sizeof(double));

            return 0;
        }

        else
            close(fd[i][1]);

    }

    wait(NULL);
    double sum= 0.0;
    fflush(stdout);
    for (int i=0; i<n; i++) {
        double buf;
        read(fd[i][0], &buf, sizeof(double));
        sum+= buf;
    }

    gettimeofday(&end, NULL);
    
    printf("%.8lf\n", sum);
    printf("Time: %.3lfs\n", (double)((end.tv_sec - start.tv_sec) * 1000 + 
        (double)(end.tv_usec - start.tv_usec) / 1000) / 1000.0);
    return 0;
}