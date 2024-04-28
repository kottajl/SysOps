#include "lib.h"
#include <stdio.h>
#include <time.h>

int my_id, my_socket;
unsigned short my_port;

void orderStop () {
    printf("%c", 13);
    
    if (my_id == (int)getpid())
        return;

    struct message my_message;

    my_message.mtype= STOP;
    my_message.msender= my_id;
    sendMessage(&my_message, my_socket, my_id);

    if (recv(my_socket, &my_message, sizeof(struct message), 0) == -1) {
        perror("Error in recv function");
        _exit(-1);
    }

    if (shutdown(my_socket, SHUT_RDWR) == -1) {
        perror("Error in shutdown function");
        _exit(-1);
    }

    if (close(my_socket) == -1) {
        perror("Error in closing socket");
        _exit(-1);
    }

    printf(" Disconnected!\n"); fflush(stdout);
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
    
    return FD_ISSET(STDIN_FILENO, &readfds);
}


// MAIN -----------------------------------------------------

int main (int argc, char** argv) {
    if (argc != 4) {
        fprintf(stderr, "Wrong number of arguments!\n");
        return -1;
    }

    /// argv[1] - name
    if (strlen(argv[1]) >= 50) {
        fprintf(stderr, "Name is too long (should be less then 50 characters)!\n");
        return -1;
    }
    /// argv[2] - IPv4
    sscanf(argv[3], "%hu", &my_port);

    printf("Client starts - hello %s\n", argv[1]); fflush(stdout);
    my_id= (int)getpid();


    my_socket= socket(AF_INET, SOCK_STREAM, 0);
    if (my_socket == -1) {
        perror("Error in creating socket");
        _exit(-1);
    }

    atexit(orderStop);
    signal(SIGINT, orderStop);

    struct sockaddr_in my_sockaddr;
    my_sockaddr.sin_family= AF_INET;    // IPv4
    my_sockaddr.sin_port= htons(my_port);
    my_sockaddr.sin_addr.s_addr= inet_addr(argv[2]);

    if (connect(my_socket, (struct sockaddr*) &my_sockaddr, sizeof(struct sockaddr_in)) == -1) {
        perror("Error in connecting to the server");
        exit(-1);
    }
    /// i am connected now

    char input[MAX_MSG_LEN + 12];
    struct message my_message;

    if (recv(my_socket, &my_message, sizeof(struct message), 0) != -1) {
        if (my_message.mtype != INIT) {
            fprintf(stderr, "Initing client has not worked out well...");
            exit(-1);
        }

        my_id= my_message.mreceiver;
        printf("My id: %d\n", my_id); fflush(stdout);
    }
    

    while (1) {

        my_message.mreceiver= -1;
        my_message.msender= my_id;
        my_message.mtype= 0;

        /// checking msgs from server
        if (recv(my_socket, &my_message, sizeof(struct message), MSG_DONTWAIT) != -1) {    // if I've got something
            char my_time[35];

            switch (my_message.mtype) {

                case STOP:
                    printf("\nServer is stopping\n");
                    return 0;       // orderStop();
                
                case LIST:
                    printf("\n%s\n", my_message.mtext);
                    continue;
                
                case TO_ONE:
                case TO_ALL:
                    sscanf(my_message.mtime_s, "%[^\n]", my_time);
                    printf("\nAt %s received text from client %d:\n%s\n", my_time, 
                            my_message.msender, my_message.mtext);
                    continue;
                
                case PING:
                    my_message.msender= my_id;
                    sendMessage(&my_message, my_socket, my_id);
                    continue;
            }
        }

        /// checking my msgs
        if (!checkInput())
            continue;

        printf("\n");
        fgets(input, MAX_MSG_LEN + 11, stdin);
        if ((strlen(input) > 0) && (input[strlen(input) - 1] == '\n'))
            input[strlen(input) - 1]= '\0';


        if (strcmp(input, "LIST") == 0) {            
            my_message.mtype= LIST;
            sendMessage(&my_message, my_socket, my_id);
        }

        else if (strncmp(input, "2ALL", 4) == 0) {
            char message_text[MAX_MSG_LEN];
            if (sscanf(input, "%*s %[^\n]", message_text) == EOF) {
                printf("Wrong input format - try '2ALL <text>'");
                continue;
            }

            my_message.mtype= TO_ALL;
            strcpy(my_message.mtext, message_text);
            sendMessage(&my_message, my_socket, my_id);
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
            sendMessage(&my_message, my_socket, my_id);
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