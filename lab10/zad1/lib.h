#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include <pwd.h>
#include <time.h>
#include <poll.h>

// max values
#define MAX_MSG_LEN 201
#define MAX_TIME_LEN 35
#define MAX_NO_CLIENTS 16

// orders
// #define NUM_OF_ORDERS 5
#define STOP 1
#define LIST 2
#define TO_ALL 3
#define TO_ONE 4
#define INIT 5
#define PING 9


struct message {
    long mtype;
    char mtext[MAX_MSG_LEN];
    int msender;                // some info about sender
    int mreceiver;              // some info about receiver
    char mtime_s[MAX_TIME_LEN];
};


#define SERVER 0

void sendMessage (struct message* my_message, int socket, int id) {

    if (send(socket, my_message, sizeof(struct message), 0) == -1) {
        if (id == SERVER)
            perror("Server: Error in sending message");
        else {
            fprintf(stderr, "Client %d: ", id);
            perror("Error in sending message");
        }
        
        _exit(-1);
    }
}


