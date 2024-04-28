#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <string.h>

const char *fifo_path= "/tmp/temp_fifo";


int main (int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Wrong number of arguments!\n");
        return -1;
    }
    
    struct timeval start, end;
    double dx= atof(argv[1]);
    int n= atoi(argv[2]);

    if (mkfifo(fifo_path, 0666) == -1) {
        perror("Error in making fifo");
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
            double d1= dx * tasks_sum[i];
            double d2= dx * (tasks_sum[i] + tasks[i]);
            char arg1[21], arg2[21], arg3[21], arg4[12];
            int child_fifo= open(fifo_path, O_WRONLY);

            sprintf(arg1, "%lf", d1);
            sprintf(arg2, "%lf", d2);
            sprintf(arg3, "%lld", tasks[i]);
            sprintf(arg4, "%d", child_fifo);
            execl("./count", "./count", arg1, arg2, arg3, arg4, NULL);

            perror("Problem with execl");
            return -1;
        }
    }

    int fifo= open(fifo_path, O_RDONLY);
    if (fifo == -1) {
        perror("Error in opening fifo (read mode)");
        return -1;
    }

    double sum= 0.0;
    for (int i=0; i<n; i++) {
        double buf;
        int err;

        if ((err= read(fifo, &buf, sizeof(double))) == -1) {
            fprintf(stderr, "i=%d: ", i);
            perror("Error in reading from fifo");
            return -1;
        }
        
        sum+= buf;
    }

    gettimeofday(&end, NULL);

    printf("%.8lf\n", sum);
    printf("Time: %.3lfs\n", (double)((end.tv_sec - start.tv_sec) * 1000 + 
        (double)(end.tv_usec - start.tv_usec) / 1000) / 1000.0);

    close(fifo);
    if (unlink(fifo_path) != 0) {
        perror("Error in removing fifo");
        return -1;
    }
    return 0;
}