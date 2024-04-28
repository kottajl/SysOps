#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

int main (int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments!\n");
        return -1;
    }

    int n= atoi(argv[1]);
    if (n < 0) {
        fprintf(stderr, "Can't do for negative argument!\n");
        return -1;
    }

    for (int i=0; i<n; i++) {
        pid_t pid= fork();
        if (pid == 0) {
            printf("My PID: %d, Parent PID: %d\n", (int)getpid(), (int)getppid());
            return 0;
        }
        wait(NULL);
    }

    printf("%s\n", argv[1]);
    return 0;
}