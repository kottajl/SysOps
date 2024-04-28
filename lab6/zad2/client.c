#include "lib.h"
#include <stdio.h>
#include <time.h>

const int msg_size= sizeof(struct message);
mqd_t server_queue, my_queue;
int my_id;


void orderStop () {
    if (my_id == (int)getpid())
        return;
    
    struct message my_message;

    my_message.mtype= STOP;
    my_message.msender= my_id;
    sendMessage(&my_message, server_queue, STOP, my_id);

    if (mq_close(my_queue) == -1) {
        perror("Error in closing server queue");
        _exit(1);
    }

    char temp[30];
    sprintf(temp, "/client_%d_queue", getpid());

    if (mq_unlink(temp) == -1) {
        perror("Error in deleting server queue");
        _exit(1);
    }

    printf(" Disconnected!\n");
    _exit(0);
}

int checkInput () {
    struct timeval tv;
    fd_set readfds;
    
    tv.tv_sec= 0;
    tv.tv_usec= 500000;                 // 0.5 sec
    
    FD_ZERO(&readfds);                  // czyszczenie zestawu
    FD_SET(STDIN_FILENO, &readfds);     // dodanie STDIN_FILENO do zestawu
    
    select(STDIN_FILENO + 1, &readfds, NULL, NULL, &tv);
    
    return FD_ISSET(STDIN_FILENO, & readfds);
}

// MAIN -----------------------------------------------------

int main () {
    printf("Client starts\n"); fflush(stdout);

    my_id= (int)getpid();

    atexit(orderStop);
    signal(SIGINT, orderStop);

    struct mq_attr attr_temple;
    attr_temple.mq_flags= 0;
    attr_temple.mq_maxsmg= MAX_NO_MSG;
    attr_temple.mq_msgsize= msg_size;
    attr_temple.mq_curmsgs= 0;  // current no messages

    struct message my_message;

    server_queue= mq_open("/server_queue", O_RDONLY, 0, &attr_temple);
    if (server_queue == -1) {
        perror("Error in opening server queue");
        return -1;
    }

    char temp[30];
    sprintf(temp, "/client_%d_queue", getpid());
    my_queue= mq_open(temp, O_WRONLY | O_CREAT | O_NONBLOCK, S_IRUSR | S_IWUSR, &attr_temple);
    if (my_queue == -1) {
        perror("Error in creating client queue");
        return -1;
    }

    // letting know that i'm alive to server
    my_message.mtype= INIT;
    my_message.msender= my_key;                         // passing key to server
    sendMessage(&my_message, server_queue, INIT, my_id);
    if (mq_receive(my_queue, (char*)(&my_message), msg_size, NULL) < 0 ||
        my_message.mtype != INIT) {
        
        perror("Error in receiving INIT message in client");
        return -1;
    }
    my_id= my_message.mreceiver;
    printf("My id: %d\n", my_id); fflush(stdout);


    // start reading inputs
    my_message.msender= my_id;
    char input[MAX_MSG_LEN + 12];

    printf("Ready to work!\n");

    unsigned int priop;
    while (1) {
        my_message.mreceiver= -1;
        my_message.msender= my_id;

        // check if need to stop
        if (mq_receive(my_queue, (char*)(&my_message), msg_size, NULL) < 0) {
            
            printf("\nServer is stopping\n");
            return 0;       // orderStop();
        }

        if(mq_receive(q_id, (char *) msg, sizeof(message), type) == -1) {
            printf("Error while receiving message!\n");
            exit(1);
        }
    }



    return 0;
}