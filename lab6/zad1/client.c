#include "lib.h"
#include <stdio.h>
#include <time.h>

const int msg_size= sizeof(struct message) - sizeof(long);
int server_queue, my_queue;
int my_id;

void orderStop () {
    if (my_id == (int)getpid())
        return;
    
    struct message my_message;

    my_message.mtype= STOP;
    my_message.msender= my_id;
    sendMessage(&my_message, server_queue, my_id);

    if (msgrcv(my_queue, &my_message, msg_size, STOP, 0) == -1) {
        perror("Error in stopping client");
        _exit(1);
    }

    if (msgctl(my_queue, IPC_RMID, NULL) == -1) {
        perror("Error in deleting client queue");
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

    key_t server_key, my_key;
    struct message my_message;

    server_key= ftok(getHomePath(), SERVER_KEY_ID);
    if (server_key == -1) {
        perror("Error in generating server key");
        return -1;
    }
    server_queue= msgget(server_key, 0);
    if (server_queue == -1) {
        perror("Error in opening queue");
        return -1;
    }

    my_key= ftok(getHomePath(), CLIENT_KEY_ID);
    if (my_key == -1) {
        perror("Error in generating client key!");
        return -1;
    }
    my_queue= msgget(my_key, IPC_CREAT | IPC_EXCL | 0666);
    if (my_queue == -1) {
        perror("Error in opening queue");
        return -1;
    }


    // letting know that i'm alive to server
    my_message.mtype= INIT;
    my_message.msender= my_key;                         // passing key to server
    sendMessage(&my_message, server_queue, my_id);
    if (msgrcv(my_queue, &my_message, msg_size, INIT, 0) == -1) {
        perror("Error in receiving INIT message in client");
        return -1;
    }
    my_id= my_message.mreceiver;
    printf("My id: %d\n", my_id); fflush(stdout);


    // start reading inputs
    my_message.msender= my_id;
    char input[MAX_MSG_LEN + 12];

    printf("Ready to work!\n");

    while (1) {
        my_message.mreceiver= -1;
        my_message.msender= my_id;

        // check if need to stop
        if (msgrcv(my_queue, &my_message, msg_size, STOP, IPC_NOWAIT) != -1) {
            printf("\nServer is stopping\n");
            return 0;       // orderStop();
        }

        // check if got list from server
        if (msgrcv(my_queue, &my_message, msg_size, LIST, IPC_NOWAIT) != -1) {
            printf("\n%s\n", my_message.mtext);
            continue;
        }

        // check if got message from another client
        if (msgrcv(my_queue, &my_message, msg_size, TO_ONE, IPC_NOWAIT) != -1 ||
            msgrcv(my_queue, &my_message, msg_size, TO_ALL, IPC_NOWAIT) != -1) {
            
            char my_time[35];
            sscanf(my_message.mtime_s, "%[^\n]", my_time);
            // my_time[strlen(my_time) - 1]= '\0';                         // deleting '\n'
            printf("\nAt %s received text from client %d:\n%s\n", my_time, 
                    my_message.msender, my_message.mtext);
            continue;
        }

        // read input
        if (!checkInput())
            continue;
        
        printf("\n");
        fgets(input, MAX_MSG_LEN + 11, stdin);
        if ((strlen(input) > 0) && (input[strlen(input) - 1] == '\n'))
            input[strlen(input) - 1]= '\0';

        if (strcmp(input, "LIST") == 0) {            
            my_message.mtype= LIST;
            sendMessage(&my_message, server_queue, my_id);
        }

        else if (strncmp(input, "2ALL", 4) == 0) {
            char message_text[MAX_MSG_LEN];
            if (sscanf(input, "%*s %[^\n]", message_text) == EOF) {
                printf("Wrong input format - try '2ALL <text>'");
                continue;
            }

            my_message.mtype= TO_ALL;
            strcpy(my_message.mtext, message_text);
            sendMessage(&my_message, server_queue, my_id);
        }

        else if (strncmp(input, "2ONE", 4) == 0) {
            int receiver_id;
            char message_text[MAX_MSG_LEN];
            if (sscanf(input, "%*s %d %[^\n]", &receiver_id, message_text) == EOF) {
                printf("Wrong input format - try '2ONE <receiver_id> <text>'");
                continue;
            }

            my_message.mtype= TO_ONE;
            strcpy(my_message.mtext, message_text);
            my_message.mreceiver= receiver_id;
            sendMessage(&my_message, server_queue, my_id);
        }

        else if (strcmp(input, "STOP") == 0)
            return 0;       // orderStop()

        else if (strcmp(input, "") == 0)
            continue;

        else
            printf("Unknown command\n");
        
    }

    return 0;
}