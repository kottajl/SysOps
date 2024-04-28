#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <time.h>

int sender_pid, received_mode;

void sendResponse () {
    kill(sender_pid, SIGUSR1);
}

void handler (int sig, siginfo_t *info, void *ucontext) {
    sender_pid= info->si_pid;

    union sigval my_sigval= info->si_value;
    received_mode= my_sigval.sival_int;
}



int main () {
    printf("My PID is %d\n\n", (int)getpid()); fflush(stdout);
    sender_pid= -1;
    received_mode= -1;

    // signal(SIGUSR1, received);
    struct sigaction act;
    act.sa_flags= SA_SIGINFO;
    act.sa_sigaction= handler;
    sigaction(SIGUSR1, &act, NULL);

    // mask: halt all signals expect SIGUSR1 and SIGINT (ctrl + c)
    sigset_t set;
    sigfillset(&set);
    sigdelset(&set, SIGUSR1);
    sigdelset(&set, SIGINT);

    time_t t;
    int mode_changes= 0;
    int last_mode= 0;
    int child_pid= -1;

    while (1) {
        sigsuspend(&set);

        if (last_mode != received_mode)
            mode_changes++;
        
        if (last_mode == 4) {
            kill(child_pid, SIGINT);
            printf("\n"); fflush(stdout);
        }

        switch (received_mode) {
            case 1:
                for (int i=1; i<=100; i++)
                    printf("%d ", i);
                printf("\n\n"); fflush(stdout);
                break;

            case 2:
                time(&t);
                printf("It's %s\n", ctime(&t)); fflush(stdout);
                break;

            case 3:
                printf("Number of mode changes: %d.\n\n", mode_changes); fflush(stdout);
                break;

            case 4:
                child_pid= fork();
                if (child_pid == 0) {
                    while (1) {
                        time(&t);
                        printf("It's %s", ctime(&t)); fflush(stdout);
                        sleep(1);
                    }
                }
                break;

            case 5:
                sendResponse();
                printf("The end of catcher.\n");
                return 0;

            default:
                continue;
        }

        sendResponse();
        last_mode= received_mode;
    }

}