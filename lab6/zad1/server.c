#include "lib.h"
#include <sys/stat.h>
#include <fcntl.h>

const int msg_size= sizeof(struct message) - sizeof(long);
struct client_info* clients_r;      // global reference to clients
int my_queue;
int my_file;
time_t rawtime;

struct client_info {
    int id;
    int queue;
    int availability;       // 0 - free, 1 - occupied
};

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
    printf(" Server is closing...\n");
    close(my_file);

    struct message my_message;
    my_message.mtype= STOP;

    for (int i=0; i<MAX_NO_CLIENTS; i++) {
        if (clients_r[i].availability == 1) {
            sendMessage(&my_message, clients_r[i].queue, SERVER);

            if (msgrcv(my_queue, &my_message, msg_size, STOP, 0) < 0) {
                perror("Error in stopping client");
                _exit(1);
            }

            clients_r[i].availability= 0;
            clients_r[i].id= -1;
            sendMessage(&my_message, clients_r[i].queue, SERVER);
            clients_r[i].queue= -1;
        }
    }

    if (msgctl(my_queue, IPC_RMID, NULL) == -1) {
        perror("Error in deleting server queue");
        _exit(1);
    }

    printf("Server has been closed.\n");
    _exit(0);
}


// MAIN -----------------------------------------------------

int main () {
    printf("Server starts\n"); fflush(stdout);

    atexit(serverEnd);
    signal(SIGINT, serverEnd);

    key_t my_key;

    my_key= ftok(getHomePath(), SERVER_KEY_ID);
    if (my_key == -1) {
        perror("Error in generating key");
        return -1;
    }
    msgctl(msgget(my_key, 0), IPC_RMID, NULL);

    my_queue= msgget(my_key, IPC_CREAT | IPC_EXCL | 0666);
    if (my_queue == -1) {
        perror("Error in generating queue");
        return -1;
    }

    // making clients tab
    struct client_info clients[MAX_NO_CLIENTS];
    for (int i=0; i<MAX_NO_CLIENTS; i++)
        clients[i].availability= 0;
    clients_r= clients;

    // making file
    my_file= open("server_logs.txt", O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (my_file < 0) {
        perror("Error in opening file");
        return -1;
    }

    // things with clients
    struct message received_message;
    int next_id= 1;

    printf("Ready to work!\n"); fflush(stdout);

    while (1) {
        if (msgrcv(my_queue, &received_message, msg_size, -(NUM_OF_ORDERS + 1), 0) < 0) {
            perror("Error in receiving message in server");
            return -1;
        }

        // setting time
        time(&rawtime);
        //received_message.mtime= localtime(&rawtime);
        strcpy(received_message.mtime_s, ctime(&rawtime));

        int it;
        char buf[MAX_MSG_LEN], little_buf[16];

        switch (received_message.mtype) {

            case INIT:
                it= findFirstAvaliable(clients);
                if (it < 0) {
                    fprintf(stderr, "Too many clients!\n");
                    return -1;
                }

                clients[it].availability= 1;
                clients[it].id= next_id++;
                clients[it].queue= msgget(received_message.msender, 0);
                if (clients[it].queue < 0) {
                    fprintf(stderr, "Error in opening client %d queue: ", clients[it].id);
                    perror("");
                    return -1;
                }

                received_message.msender= clients[it].id;
                received_message.mreceiver= clients[it].id;                   // passing id to client
                sendMessage(&received_message, clients[it].queue, SERVER);
                break;


            case LIST:
                it= getIteratorOfClient(clients, received_message.msender);
                buf[0]= '\0';
                strcat(buf, "Active clients:\n");
                for (int i=0; i<MAX_NO_CLIENTS; i++) {
                    if (clients[i].availability == 1) {
                        sprintf(little_buf, " > Client %d\n", clients[i].id);
                        strcat(buf, little_buf);
                    }
                }
                
                strcpy(received_message.mtext, buf);
                // printf("%s\n", received_message.mtext); fflush(stdout);
                // printf("it = %d\n", it);
                // printf("len(.mtext) = %d\n", (int)strlen(received_message.mtext));
                sendMessage(&received_message, clients[it].queue, SERVER);
                break;
            

            case TO_ALL:
                for (int i=0; i<MAX_NO_CLIENTS; i++) {
                    if (clients[i].availability == 1 && clients[i].id != received_message.msender) {
                        // printf("- client %d != sender client %d\n", clients[i].id, received_message.msender);
                        sendMessage(&received_message, clients[i].queue, SERVER);
                    }    
                }
                printf("\n"); fflush(stdout);
                break;


            case TO_ONE:
                // received_message.mreceiver -> receiver_id
                it= getIteratorOfClient(clients, received_message.mreceiver);
                if (it < 0) {
                    printf("Client %d is not available or not existing.\n", received_message.mreceiver);
                    break;
                }

                sendMessage(&received_message, clients[it].queue, SERVER);
                break;
            

            case STOP:
                it= getIteratorOfClient(clients, received_message.msender);
                clients[it].availability= 0;
                clients[it].id= -1;
                
                sendMessage(&received_message, clients[it].queue, SERVER);
                clients[it].queue= -1;
                break;
            

            default:
                fprintf(stderr, "Some strange error has occured...\n");
                break;
        }

        writeToFile(&received_message);
    }

    return 0;
}