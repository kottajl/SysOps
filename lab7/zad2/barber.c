#include "mylib.h"

int my_id, P;
sem_t *barbers_sem, *armchairs_sem, *clients_sem;
struct client_info* waitingroom_spots;

void clean_trash () {

    if (sem_close(barbers_sem) == -1) {
        perror("barber: Error in closing barbers semaphore");
        _exit(-1);
    }
    if (sem_close(armchairs_sem) == -1) {
        perror("barber: Error in closing armchairs semaphore");
        _exit(-1);
    }
    if (sem_close(clients_sem) == -1) {
        perror("barber: Error in closing clients semaphore");
        _exit(-1);
    }
    if (munmap(waitingroom_spots, (P+1) * sizeof(struct client_info)) == -1) {
        perror("barber: Error in closing waiting room shared memory segment");
        _exit(-1);
    }

    printf("\nBarber no %d is outta here. \n", my_id);
}

void sigint_handler () {
    exit(0);
}


int main (int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Wrong number of arguments!\n");
        return -1;
    }
    my_id= atoi(argv[1]);
    P= atoi(argv[2]);
    printf("Barber no %d is alive!\n", my_id); fflush(stdout);

    barbers_sem= get_semaphore(BARBER_FILE_NAME, 0, "barbers", 1);
    armchairs_sem= get_semaphore(ARMCHAIR_FILE_NAME, 0, "armchair", 1);
    clients_sem= get_semaphore(CLIENT_FILE_NAME, 0, "clients", 1);

    int waitingroom_shm= get_shared_memory(WAITINGROOM_FILE_NAME, 1);
    waitingroom_spots= mmap(NULL, (P+1) * sizeof(struct client_info), PROT_WRITE | PROT_READ, MAP_SHARED, waitingroom_shm, 0);
    if (waitingroom_spots == (void*)-1) {
        perror("barber: Error in adding shared memory segment to address space");
        return -1;
    }

    atexit(clean_trash);
    signal(SIGINT, sigint_handler);

    while (1) {

        if (sem_wait(armchairs_sem) == -1) {
            perror("Error in trying to find free armchair");
            exit(-1);
        }
        // now I have an armchair for my next client

        usleep(1000 * 200);
        if (sem_wait(clients_sem) == -1) {
            perror("Error in trying to get a client");
            exit(-1);
        }
        // now I have a client

        if (sem_wait(barbers_sem) == -1) {
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

        if (sem_post(barbers_sem) == -1) {
            perror("Error in incrementing number of free barbers");
            exit(-1);
        }
        if (sem_post(armchairs_sem) == -1) {
            perror("Error in trying to free armchair I've used");
            exit(-1);
        }
        // now me and the armchair are free

        // killing my client (after the payment of course)
        kill(my_client_pid, SIGUSR1);
    }

    return 0;
}