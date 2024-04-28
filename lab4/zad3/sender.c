#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>

void emptyHandler () {
    return;
}

int checkInput (const char* input) {
    if (strcmp(input, "1") == 0 || 
        strcmp(input, "2") == 0 || 
        strcmp(input, "3") == 0 || 
        strcmp(input, "4") == 0 || 
        strcmp(input, "5") == 0) return 0;
    
    printf("Can't resolve %s - skipping.\n", input); fflush(stdout);
    return -1;
}

int main (int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Not enough arguments!\n");
        return -1;
    }

    const int catcher_pid= atoi(argv[1]);
    signal(SIGUSR1, emptyHandler);

    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);
    sigdelset(&set, SIGINT);

    for (int it=2; it<argc; it++) {
        if (checkInput(argv[it]) < 0)
            continue;
        
        const int mode= atoi(argv[it]);

        union sigval value;
        value.sival_int= mode;
        // printf("> I'm %d and I'm about to send a signal.\n", (int)getpid()); fflush(stdout);
        sigqueue(catcher_pid, SIGUSR1, value);
        sigsuspend(&set);

        if (mode == 5)
            break;
    }

    return 0;
}