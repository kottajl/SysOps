#include "lib.h"
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

struct client_info {
    int id;
    int socket;
    int availability;       // 0 - free, 1 - occupied
};

// socklen_t sockaddr_in_size= sizeof(struct sockaddr_in);     // sizeof in variable to accept() function
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
    printf("%c", 13);
    printf("Server is closing...\n");
    close(my_file);

    pthread_cancel(pinger);

    struct message my_message;
    my_message.mtype= STOP;

    for (int i=0; i<MAX_NO_CLIENTS; i++) {
        if (clients_r[i].availability == 1) {
            sendMessage(&my_message, clients_r[i].socket, SERVER);

            if (recv(clients_r[i].socket, &my_message, sizeof(struct message), 0) == -1) {
                perror("Error in stopping client");
                _exit(-1);
            }

            clients_r[i].availability= 0;
            clients_r[i].id= -1;
            sendMessage(&my_message, clients_r[i].socket, SERVER);

            if (shutdown(clients_r[i].socket, SHUT_RDWR) == -1) {
                perror("Error in shutdown function");
                _exit(-1);
            }

            if (close(clients_r[i].socket) == -1) {
                perror("Error in closing socket");
                _exit(-1);
            }

            clients_r[i].socket= -1;
        }
    }

    if (shutdown(my_socket, SHUT_RDWR) == -1) {
        perror("Error in shutdown function");
        _exit(-1);
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
            sendMessage(&my_message, clients_r[i].socket, SERVER);
        }
        pthread_mutex_unlock(&clients_mutex);

        sleep(4);
        for (int i=0; i<MAX_NO_CLIENTS; i++)
            if (is_pinged_back[i] == 0) {
                printf("Client %d kicked out for not responding\n", clients_r[i].id); fflush(stdout);

                is_pinged_back[i]= 1;
                clients_r[i].availability= 0;
                clients_r[i].id= -1;

                if (shutdown(clients_r[i].socket, SHUT_RDWR) == -1) {
                    perror("Error in shutdown function");
                    _exit(-1);
                }
                if (close(clients_r[i].socket) == -1) {
                    perror("Error in closing socket");
                    _exit(-1);
                }

                clients_r[i].socket= -1;
            }

    }

    return arg;
}


int checkIncome (fd_set* my_fdset) {
    struct timeval tv;
    tv.tv_sec= 0;
    tv.tv_usec= 500000;                     // 0.5 sec

    FD_ZERO(my_fdset);                     // czyszczenie zestawu

    int max_socket= 0;

    for (int i=0; i<MAX_NO_CLIENTS; i++) {  // dodawanie aktywnych socketow
        if (clients_r[i].availability == 1) {
            FD_SET(clients_r[i].socket, my_fdset);
            if (clients_r[i].socket > max_socket)
                max_socket= clients_r[i].socket;
        }
    }

    int select_status= select(max_socket + 1, my_fdset, NULL, NULL, &tv);
    if (select_status == -1) {
        perror("Error in select in function checkIncome");
        exit(-1);
    }

    return select_status;
}


void parseInput (struct message* received_message) {
    
    // setting time
    time(&rawtime);
    strcpy(received_message->mtime_s, ctime(&rawtime));

    int it;
    char buf[MAX_MSG_LEN], little_buf[16];


    switch (received_message->mtype) {

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
            sendMessage(received_message, clients_r[it].socket, SERVER);
            break;
        

        case TO_ALL:
            printf("Received 2ALL message from client %d ... ", received_message->msender); fflush(stdout);
            for (int i=0; i<MAX_NO_CLIENTS; i++) {
                if (clients_r[i].availability == 1 && clients_r[i].id != received_message->msender) {
                    // printf("- client %d != sender client %d\n", clients[i].id, received_message.msender);
                    sendMessage(received_message, clients_r[i].socket, SERVER);
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

            sendMessage(received_message, clients_r[it].socket, SERVER);
            break;
        

        case STOP:
            printf("Received STOP message from client %d ... ", received_message->msender); fflush(stdout);
            it= getIteratorOfClient(clients_r, received_message->msender);
            clients_r[it].availability= 0;
            clients_r[it].id= -1;
            
            sendMessage(received_message, clients_r[it].socket, SERVER);

            if (shutdown(clients_r[it].socket, SHUT_RDWR) == -1) {
                perror("Error in shutdown function");
                _exit(-1);
            }
            if (close(clients_r[it].socket) == -1) {
                perror("Error in closing socket");
                _exit(-1);
            }
            clients_r[it].socket= -1;
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

    my_socket= socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);     // nonblock socket
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

    if (listen(my_socket, 10) == -1) {
        perror("blad listen");
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

        // printf("Before checking new client\n"); fflush(stdout);
        
        /// CHECKING NEW CLIENT
        struct sockaddr_in new_cl_sockaddr;
        socklen_t sockaddr_in_size= sizeof(struct sockaddr_in);

        int new_client= accept(my_socket, (struct sockaddr*) &new_cl_sockaddr, &sockaddr_in_size);
        if (new_client != -1) {

            int it= findFirstAvaliable(clients);
            if (it < 0) {
                fprintf(stderr, "Too many clients!\n");
                exit(-1);
            }


            clients_r[it].availability= 1;
            clients_r[it].id= next_id++;
            clients_r[it].socket= new_client;

            time(&rawtime);
            strcpy(received_message.mtime_s, ctime(&rawtime));
            received_message.mtype= INIT;
            received_message.msender= clients[it].id;
            received_message.mreceiver= clients[it].id;                   // passing id to client
            sendMessage(&received_message, clients[it].socket, SERVER);
            
            printf("New client connected! - id: %d\n", clients[it].id); fflush(stdout);
            writeToFile(&received_message);

            continue;
        }

        // printf("After checking new client\n"); fflush(stdout);

        /// CHECKING INCOMING MESSAGES
        pthread_mutex_lock(&clients_mutex);
        
        fd_set my_fdset;
        if (checkIncome(&my_fdset) > 0) {

            for (int i=0; i<MAX_NO_CLIENTS; i++) {

                if (clients[i].availability == 0 || !FD_ISSET(clients[i].socket, &my_fdset))
                    continue;
                
                if (recv(clients[i].socket, &received_message, sizeof(struct message), 0) == -1) {
                    perror("Error in recv function");
                    exit(-1);
                }

                parseInput(&received_message);

                if (received_message.mtype != PING)
                    writeToFile(&received_message);
            }

        }
        pthread_mutex_unlock(&clients_mutex);
        
        usleep(1000 * 100);
    }

    return 0;
}
