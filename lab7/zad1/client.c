#include "mylib.h"
#include <stdio.h>

struct client_info* waitingroom_spots;

void clean_trash () {
    shmdt(waitingroom_spots);
}

void sigint_handler () {
    exit(0);
}

int main (int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments!\n");
        return -1;
    }
    int my_hairstyle= atoi(argv[1]);
    printf("\nHello, I'm %d and I want hairstyle no %d\n", (int)getpid(), my_hairstyle);
    fflush(stdout);

    int clients_sem= get_semaphore(get_client_key(), "clients", 1);
    int waitingroom= get_shared_memory(get_waitingroom_key(), 0, 1);
    waitingroom_spots= shmat(waitingroom, NULL, 0);
    int P= waitingroom_spots[0].pid;
    int first_queue_it= waitingroom_spots[0].hairstyle;

    atexit(clean_trash);
    signal(SIGINT, sigint_handler);


    // WRITING QUEUE ON STDOUT
    printf("As client I'm looking on waitroom and see:\n");
    for (int i=0; i <= P; i++) {
        int temp_it= (waitingroom_spots[0].hairstyle + i) % (P+1);
        if (temp_it == 0)
            continue;
        
        printf(". %d %d\n", waitingroom_spots[temp_it].pid, waitingroom_spots[temp_it].hairstyle);
        fflush(stdout);
    }


    int free_spots= P - semctl(clients_sem, 0, GETVAL, NULL);
    if (free_spots <= 0) {
        printf("Sadly, there is no place for me. Sayonara then!\n\n");
        return 0;
    }
    printf("\n"); fflush(stdout);

    struct sembuf increment_op[1];
    increment_op[0].sem_num= 0;
    increment_op[0].sem_op= 1;
    increment_op[0].sem_flg= 0;

    int my_seat_it= (first_queue_it + (P - free_spots)) % (P+1);
    if (my_seat_it < first_queue_it)
        my_seat_it++;                       // because iterator = 0 isn't for use
    
    waitingroom_spots[my_seat_it].pid= (int)getpid();
    waitingroom_spots[my_seat_it].hairstyle= my_hairstyle;

    if (semop(clients_sem, increment_op, 1) == -1) {
        perror("Error in trying to sit in waitroom");
        exit(-1);
    }

    // DEBUG v
    // printf("As client I'm looking on waitroom and see:\n");
    // for (int i=0; i<=P; i++)
    //     printf(". %d %d\n", waitingroom_spots[i].pid, waitingroom_spots[i].hairstyle);
    // printf("\n"); fflush(stdout);
    // DEBUG ^

    pause();

    return 0;
}