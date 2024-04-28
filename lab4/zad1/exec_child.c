#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>

void ignore () {
    signal(SIGUSR1, SIG_IGN);
}

void fnHandler (int signum) {
    printf("I'm %d and I've recieved signal %d", (int)getpid(), signum);
}

void handler () {
    signal(SIGUSR1, fnHandler);
}

void mask () {
    sigset_t newmask, oldmask;
    sigemptyset(&newmask);
    sigaddset(&newmask, SIGUSR1);

    if (sigprocmask(SIG_BLOCK, &newmask, &oldmask) == -1) {
        perror("Error in setting mask on signals");
        exit(1);
    }
}

void pending () {
    sigset_t set;
    sigpending(&set);
    if (sigismember(&set, SIGUSR1) == 1)
        printf("Waiting signal is visible in process %d.", (int)getpid());
    else
        printf("Waiting signal is NOT visible in process %d.", (int)getpid());

    fflush(stdout);
}


int main (int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments!\n");
        return -1;
    }

    int mode= -1;
    if (strcmp(argv[1], "ignore") == 0)
        mode= 0;
    else if (strcmp(argv[1], "mask") == 0)
        mode= 1;
    else if (strcmp(argv[1], "pending") == 0)
        mode= 2;
    else {
        fprintf(stderr, "Wrong argument!\n");
        return -1;
    }

    printf(" In child: "); fflush(stdout);

    switch (mode) {
        case 0:
            ignore();
            break;
        case 1:
            mask();
            break;
        case 2:
            mask();
            break;
        default:
            fprintf(stderr, "Some strange bug has appeared...\n");
            return -1;
    }

    raise(SIGUSR1);

    if (mode == 2)
        pending();
    
    printf("\n"); fflush(stdout);

    return 0;
}