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

    printf("%d ", (int)getpid());
    fflush(stdout);

    pid_t pid= fork();
    if (pid == 0)
        execl("/bin/ls", "/bin/ls", argv[1], NULL);

    wait(NULL);
    printf("\n");

    return 0;
}