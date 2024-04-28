#include "mylib.h"

int my_id;
struct client_info* waitingroom_spots;

void clean_trash () {
    printf("\nBarber no %d is outta here. \n", my_id); fflush(stdout);
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
    my_id= atoi(argv[1]);
    printf("Barber no %d is alive!\n", my_id); fflush(stdout);

    int barbers_sem= get_semaphore(get_barber_key(), "barbers", 1);
    int clients_sem= get_semaphore(get_client_key(), "clients", 1);
    int armchairs_sem= get_semaphore(get_armchair_key(), "armchair", 1);
    int waitingroom= get_shared_memory(get_waitingroom_key(), 0, 1);
    waitingroom_spots= shmat(waitingroom, NULL, 0);
    int P= waitingroom_spots[0].pid;

    atexit(clean_trash);
    signal(SIGINT, sigint_handler);

    struct sembuf decrement_op[1];
    decrement_op[0].sem_num= 0;
    decrement_op[0].sem_op= -1;
    decrement_op[0].sem_flg= 0;

    struct sembuf increment_op[1];
    increment_op[0].sem_num= 0;
    increment_op[0].sem_op= 1;
    increment_op[0].sem_flg= 0;

    while (1) {

        if (semop(armchairs_sem, decrement_op, 1) == -1) {
            perror("Error in trying to find free armchair");
            exit(-1);
        }
        // now I have an armchair for my next client

        usleep(1000 * 200);
        if (semop(clients_sem, decrement_op, 1) == -1) {
            perror("Error in trying to get a client");
            exit(-1);
        }
        // now I have a client

        if (semop(barbers_sem, decrement_op, 1) == -1) {
            perror("Error in decrementing number of free barbers");
            exit(-1);
        }

        // DEBUG v
        // printf("Barber is looking on waitroom and sees:\n");
        // for (int i=0; i<=P; i++)
        //     printf(". %d %d\n", waitingroom_spots[i].pid, waitingroom_spots[i].hairstyle);
        // printf("\n"); fflush(stdout);
        // DEBUG ^

        int first_queue_it= waitingroom_spots[0].hairstyle;
        waitingroom_spots[0].hairstyle= max ( 1, (waitingroom_spots[0].hairstyle + 1) % (P+1) );
        // changed queue first client

        int my_client_pid= waitingroom_spots[first_queue_it].pid;
        int my_client_hairstyle= waitingroom_spots[first_queue_it].hairstyle;
        waitingroom_spots[first_queue_it].pid= 0;
        waitingroom_spots[first_queue_it].hairstyle= 0;
        // my client is no longer in queue

        if (my_client_pid == 0)
            continue;

        printf("Barber no %d starts doing a haircut (style %d) for client %d\n", 
                my_id, my_client_hairstyle, my_client_pid); fflush(stdout);
        
        sleep(my_client_hairstyle + 2);         // making haircut for my client

        printf("Barber no %d has ended!\n\n", my_id);

        if (semop(barbers_sem, increment_op, 1) == -1) {
            perror("Error in incrementing number of free barbers");
            exit(-1);
        }
        if (semop(armchairs_sem, increment_op, 1) == -1) {
            perror("Error in trying to free armchair I've used");
            exit(-1);
        }
        // now I the armchair is free

        // killing my client (after the payment of course)
        kill(my_client_pid, SIGINT);
    }

    return 0;
}