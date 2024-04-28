#include "mylib.h"

int P;
sem_t* clients_sem;
struct client_info* waitingroom_spots;


void clean_trash () {

    if (sem_close(clients_sem) == -1) {
        perror("client: Error in closing clients semaphore");
        _exit(-1);
    }
    if (munmap(waitingroom_spots, (P+1) * sizeof(struct client_info)) == -1) {
        perror("client: Error in closing waiting room shared memory segment");
        _exit(-1);
    }
}


void sigint_handler () {
    printf("\nClient %d was kicked out.\n", (int)getpid()); fflush(stdout);
    exit(0);
}

void sigusr_handler () { exit(0); }



int main (int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Wrong number of arguments!\n");
        return -1;
    }
    int my_hairstyle= atoi(argv[1]);
    P= atoi(argv[2]);

    printf("\nHello, I'm %d and I want hairstyle no %d\n", (int)getpid(), my_hairstyle); fflush(stdout);

    // int barbers_sem= get_semaphore(get_barber_key(), "barbers", 1);
    clients_sem= get_semaphore(CLIENT_FILE_NAME, 0, "clients", 1);
    int waitingroom_shm= get_shared_memory(WAITINGROOM_FILE_NAME, 1);

    waitingroom_spots= mmap(NULL, (P+1) * sizeof(struct client_info), PROT_WRITE | PROT_READ, MAP_SHARED, waitingroom_shm, 0);
    if (waitingroom_spots == (void*)-1) {
        perror("client: Error in adding shared memory segment to address space");
        return -1;
    }
    
    int first_queue_it= waitingroom_spots[0].hairstyle;

    atexit(clean_trash);
    signal(SIGINT, sigint_handler);
    signal(SIGUSR1, sigusr_handler);


    // WRITING QUEUE ON STDOUT
    printf("As client I'm looking on waitroom and see:\n");
    for (int i=0; i <= P; i++) {
        int temp_it= (waitingroom_spots[0].hairstyle + i) % (P+1);
        if (temp_it == 0)
            continue;
        
        printf(". %d %d\n", waitingroom_spots[temp_it].pid, waitingroom_spots[temp_it].hairstyle);
        fflush(stdout);
    }


    int free_spots= P - get_val_of_sem(clients_sem);
    if (free_spots <= 0) {
        printf("Sadly, there is no place for me. Sayonara then!\n\n");
        return 0;
    }
    printf("\n"); fflush(stdout);

    // DEBUG v
    // printf("As client I'm looking on waitroom and see:\n");
    // for (int i=1; i<=P; i++)
    //     printf(". %d %d\n", waitingroom_spots[i].pid, waitingroom_spots[i].hairstyle);
    // printf("\n"); fflush(stdout);
    // DEBUG ^

    int my_seat_it= (first_queue_it + (P - free_spots)) % (P+1);
    if (my_seat_it < first_queue_it)
        my_seat_it++;                       // because iterator = 0 isn't for use
    
    waitingroom_spots[my_seat_it].pid= (int)getpid();
    waitingroom_spots[my_seat_it].hairstyle= my_hairstyle;

    if (sem_post(clients_sem) == -1) {
        perror("Error in trying to sit in waitroom");
        exit(-1);
    }

    pause();

    return 0;
}