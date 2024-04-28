#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h>

// max values
#define MAX_MSG_LEN 201
#define MAX_TIME_LEN 35
#define MAX_NO_CLIENTS 16

// ID's to queue
#define SERVER_KEY_ID 's'
#define CLIENT_KEY_ID getpid()

// orders
#define NUM_OF_ORDERS 5
#define STOP 1
#define LIST 2
#define TO_ALL 3
#define TO_ONE 4
#define INIT 5


struct message {
    long mtype;
    char mtext[MAX_MSG_LEN];
    int msender;                // some info about sender
    int mreceiver;              // some info about receiver
    // struct tm* mtime;
    char mtime_s[MAX_TIME_LEN];
};


char* getHomePath () {
    char* home_path= getenv("HOME");

    if (home_path == NULL)
        home_path= getpwuid(getuid())->pw_dir;

    return home_path;
}


#define SERVER 0

void sendMessage (struct message* my_message, int queue, int id) {
    if (msgsnd(queue, my_message, sizeof(struct message) - sizeof(long), IPC_NOWAIT) < 0) {
        if (id == SERVER)
            perror("Server: Error in sending message");
        else {
            fprintf(stderr, "Client %d: ", id);
            perror("Error in sending message");
        }

        _exit(1);
    }
}
