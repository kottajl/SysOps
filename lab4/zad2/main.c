#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <time.h>

void bigHandler (int sig, siginfo_t *info, void *ucontext) {
    printf("> signal number: %d\n", info->si_signo);
    printf("> signal code: %d\n", info->si_code);
    printf("> sending process ID: %d\n", (int)info->si_pid);
    printf("> my PID: %d\n", (int)getpid());
    printf("> my PPID: %d\n", (int)getppid());
    fflush(stdout);
}

void smallHandler1 (int signum) {
    printf("  > signal: %d\n", signum);
    printf("  > PID:    %d\n", (int)getpid());
    printf("  > PPID:   %d\n", (int)getppid());
    fflush(stdout);
}

int counter;

void smallHandler2 (int signum) {
    counter++;
    if (counter < 2) {
        printf("> signal: %d\n", signum);
        printf("> PID:    %d\n", (int)getpid());
        printf("> PPID:   %d\n", (int)getppid());
        fflush(stdout);
    
        pause();
    }
}


void fn1 () {
    printf("\n1. Flag= SA_SIGINFO\n");
    printf("This flag gives us a lot of useful information about received signal.\n");

    struct sigaction act;
    act.sa_flags= SA_SIGINFO;
    act.sa_sigaction= bigHandler;
    sigaction(SIGUSR1, &act, NULL);

    raise(SIGUSR1);
}

void fn2 () {
    printf("\n2. Flag= SA_RESETHAND\n");
    printf("This flag restores signal action to default after executing the handler.\n");

    int child_pid= fork();
    if (child_pid == 0) {
        struct sigaction act;
        act.sa_flags= SA_RESETHAND;
        act.sa_handler= smallHandler1;
        sigaction(SIGUSR1, &act, NULL);

        printf("- First output -\n");
        raise(SIGUSR1);

        printf("- Second output -\n");
        raise(SIGUSR1);

        exit(1);
    }

    wait(NULL);
}

void fn3 () {
    printf("\n3. Flag= SA_NODEFER\n");
    printf("With this flag we can read signal while being inside handler.\n");

    struct sigaction act;
    act.sa_flags= SA_NODEFER;
    sigemptyset(&act.sa_mask);
    act.sa_handler= smallHandler2;
    sigaction(SIGUSR1, &act, NULL);

    int child_pid= fork();

    if (child_pid != 0)
        raise(SIGUSR1);

    if (child_pid == 0) {
        sleep(1);
        kill(getppid(), SIGUSR1);
        exit(1);
    }
}

int main () {
    counter= 0;

    fn1();
    fn2();
    fn3();

    return 0;
}