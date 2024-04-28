#include "lib.h"
#include <netinet/in.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

struct client_info {
    int id;
    // int socket;
    struct sockaddr_in web_info;
    int availability;       // 0 - free, 1 - occupied
};

struct sockaddr_in new_cl_sockaddr;
struct client_info* clients_r;      // global reference to clients
unsigned short my_port;
int my_socket;
int my_file;
time_t rawtime;

pthread_t pinger;
pthread_mutex_t clients_mutex;
int is_pinged_back[MAX_NO_CLIENTS];

int findFirstAvaliable(struct client_info* clients) {
    for (int i=0; i<MAX_NO_CLIENTS; i++)
        if (clients[i].availability == 0)
            return i;
    
    return -1;
}

int getIteratorOfClient (struct client_info* clients, int id) {
    for (int i=0; i<MAX_NO_CLIENTS; i++)
        if (clients[i].id == id)
            return i;
    
    return -1;
}


void writeToFile (struct message* my_message) {
    int len;
    char buf[MAX_MSG_LEN];

    strcpy(buf, my_message->mtime_s);
    len= strlen(buf);
    write(my_file, buf, sizeof(char) * len);

    sprintf(buf, "client %d\n", my_message->msender);
    len= strlen(buf);
    write(my_file, buf, sizeof(char) * len);

    if (my_message->mtype == INIT) {
        strcpy(buf, "INIT\n");
        len= strlen(buf);
        write(my_file, buf, sizeof(char) * len);
    }
        
    else if (my_message->mtype == LIST) {
        strcpy(buf, "LIST\n");
        len= strlen(buf);
        write(my_file, buf, sizeof(char) * len);
    }
        
    else if (my_message->mtype == STOP) {
        strcpy(buf, "STOP\n");
        len= strlen(buf);
        write(my_file, buf, sizeof(char) * len);
    }
        
    else {

        if (my_message->mtype == TO_ALL)
            strcpy(buf, "TO_ALL ");

        else if (my_message->mtype == TO_ONE)
            sprintf(buf, "TO_ONE %d ", my_message->mreceiver);

        len= strlen(buf);
        write(my_file, buf, sizeof(char) * len);

        strcpy(buf, my_message->mtext);
        len= strlen(buf);
        write(my_file, buf, sizeof(char) * len);

        buf[0]= '\n';
        write(my_file, buf, sizeof(char));
    }

    buf[0]= '\n';
    write(my_file, buf, sizeof(char));
}


void serverEnd () {
    printf("%c", 13); fflush(stdout);
    printf("Server is closing...\n");
    close(my_file);

    pthread_cancel(pinger);

    struct message my_message;
    my_message.mtype= STOP;

    for (int i=0; i<MAX_NO_CLIENTS; i++) {
        if (clients_r[i].availability == 1) {
            sendMessage(&my_message, my_socket, SERVER, &clients_r[i].web_info);

            socklen_t sockaddr_in_size= sizeof(struct sockaddr_in);
            if (recvfrom(my_socket, &my_message, sizeof(struct message), 0, (struct sockaddr*) &clients_r[i].web_info, &sockaddr_in_size) == -1) {
                perror("Error in stopping client");
                _exit(-1);
            }

            clients_r[i].availability= 0;
            clients_r[i].id= -1;
            sendMessage(&my_message, my_socket, SERVER, &clients_r[i].web_info);
        }
    }

    if (close(my_socket) == -1) {
        perror("Error in closing socket");
        _exit(-1);
    }

    printf("Server has been closed.\n");
    _exit(0);
}


void* pinger_fn (void* arg) {

    struct message my_message;
    my_message.mtype= PING;

    while (1) {

        sleep(2);

        pthread_mutex_lock(&clients_mutex);
        for (int i=0; i<MAX_NO_CLIENTS; i++) {

            if (clients_r[i].availability != 1)
                continue;
            
            is_pinged_back[i]= 0;
            sendMessage(&my_message, my_socket, SERVER, &clients_r[i].web_info);
        }
        pthread_mutex_unlock(&clients_mutex);

        sleep(4);
        for (int i=0; i<MAX_NO_CLIENTS; i++)
            if (is_pinged_back[i] == 0) {
                printf("Client %d kicked out for not responding\n", clients_r[i].id); fflush(stdout);

                is_pinged_back[i]= 1;
                clients_r[i].availability= 0;
                clients_r[i].id= -1;
            }

    }

    return arg;
}


void parseInput (struct message* received_message, int* next_id) {
    
    // setting time
    time(&rawtime);
    strcpy(received_message->mtime_s, ctime(&rawtime));

    int it;
    char buf[MAX_MSG_LEN], little_buf[16];


    switch (received_message->mtype) {

        case INIT:
            it= findFirstAvaliable(clients_r);
            if (it < 0) {
                fprintf(stderr, "Too many clients!\n");
                exit(-1);
            }

            memcpy(&clients_r[it].web_info, &new_cl_sockaddr, sizeof(struct sockaddr_in));
            clients_r[it].availability= 1;
            clients_r[it].id= (*next_id)++;

            time(&rawtime);
            strcpy(received_message->mtime_s, ctime(&rawtime));
            received_message->mtype= INIT;
            received_message->msender= clients_r[it].id;
            received_message->mreceiver= clients_r[it].id;                   // passing id to client
            sendMessage(received_message, my_socket, SERVER, &clients_r[it].web_info);
            
            printf("New client connected! - id: %d\n", clients_r[it].id); fflush(stdout);
            break;


        case LIST:
            printf("Received LIST message from client %d ... ", received_message->msender); fflush(stdout);
            it= getIteratorOfClient(clients_r, received_message->msender);
            buf[0]= '\0';
            strcat(buf, "Active clients:\n");
            for (int i=0; i<MAX_NO_CLIENTS; i++) {
                if (clients_r[i].availability == 1) {
                    sprintf(little_buf, " > Client %d\n", clients_r[i].id);
                    strcat(buf, little_buf);
                }
            }
            
            strcpy(received_message->mtext, buf);
            sendMessage(received_message, my_socket, SERVER, &clients_r[it].web_info);
            break;
        

        case TO_ALL:
            printf("Received 2ALL message from client %d ... ", received_message->msender); fflush(stdout);
            for (int i=0; i<MAX_NO_CLIENTS; i++) {
                if (clients_r[i].availability == 1 && clients_r[i].id != received_message->msender) {
                    // printf("- client %d != sender client %d\n", clients[i].id, received_message.msender);
                    sendMessage(received_message, my_socket, SERVER, &clients_r[i].web_info);
                }    
            }
            break;


        case TO_ONE:
            // received_message.mreceiver -> receiver_id
            printf("Received 2ONE message from client %d ... ", received_message->msender); fflush(stdout);
            it= getIteratorOfClient(clients_r, received_message->mreceiver);
            if (it < 0) {
                printf("Client %d is not available or not existing.\n", received_message->mreceiver);
                break;
            }

            sendMessage(received_message, my_socket, SERVER, &clients_r[it].web_info);
            break;
        

        case STOP:
            printf("Received STOP message from client %d ... ", received_message->msender); fflush(stdout);
            it= getIteratorOfClient(clients_r, received_message->msender);
            clients_r[it].availability= 0;
            clients_r[it].id= -1;
            
            sendMessage(received_message, my_socket, SERVER, &clients_r[it].web_info);
            break;
        

        case PING:
            it= getIteratorOfClient(clients_r, received_message->msender);
            is_pinged_back[it]= 1;
            break;


        default:
            fprintf(stderr, "Some strange error has occured...\n");
            break;
    }

    if (received_message->mtype != PING) {
        printf("- completed.\n"); fflush(stdout);
    }
    
}

// ---------------------------------------------------------------------------------

int main (int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Wrong number of arguments!\n");
        return -1;
    }

    printf("Server starts\n"); fflush(stdout);

    sscanf(argv[1], "%hu", &my_port);

    my_socket= socket(AF_INET, SOCK_DGRAM, 0);     // nonblock socket
    if (my_socket == -1) {
        perror("Error in creating socket");
        exit(-1);
    }

    struct sockaddr_in my_sockaddr;
    my_sockaddr.sin_family= AF_INET;    // IPv4
    my_sockaddr.sin_port= htons(my_port);
    my_sockaddr.sin_addr.s_addr= inet_addr(argv[2]);
    // my_sockaddr.sin_addr.s_addr= htonl(INADDR_ANY);

    int enable = 1;
    if (setsockopt(my_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("Error in setsockopt");  // trying to reuse existing address
        exit(-1);
    }

    if (bind(my_socket, (struct sockaddr*) &my_sockaddr, sizeof(struct sockaddr_in)) == -1) {
        perror("Error in binding");
        exit(-1);
    }

    atexit(serverEnd);
    signal(SIGINT, serverEnd);

    // making clients tab
    struct client_info clients[MAX_NO_CLIENTS];
    for (int i=0; i<MAX_NO_CLIENTS; i++)
        clients[i].availability= 0;
    clients_r= clients;


    /// initializing clients mutex
    if (pthread_mutex_init(&clients_mutex, NULL) == -1) {
        perror("Error in initializing clients mutex!\n");
		exit(-1);
    }

    // making file
    my_file= open("server_logs.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (my_file < 0) {
        perror("Error in opening file");
        exit(-1);
    }
    

    for (int i=0; i<MAX_NO_CLIENTS; i++)
        is_pinged_back[i]= 1;

    /// pinger thread
    pthread_create(&pinger, NULL, pinger_fn, NULL);

    // things with clients
    struct message received_message;
    int next_id= 1;
    printf("Ready to work!\n"); fflush(stdout);

    while (1) {

        /// CHECKING INCOMING MESSAGES
        pthread_mutex_lock(&clients_mutex);

        socklen_t sockaddr_in_size= sizeof(struct sockaddr_in);
        if (recvfrom(my_socket, &received_message, sizeof(struct message), SOCK_NONBLOCK, (struct sockaddr*) &new_cl_sockaddr, &sockaddr_in_size) == -1) {
            pthread_mutex_unlock(&clients_mutex);
            continue;
        }

        parseInput(&received_message, &next_id);

        if (received_message.mtype != PING)
            writeToFile(&received_message);

        pthread_mutex_unlock(&clients_mutex);
        
        usleep(1000 * 100);
    }

    return 0;
}
