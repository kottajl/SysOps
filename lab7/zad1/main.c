#include "mylib.h"

int M;   // M barbers
int N;   // N armchairs
int P;   // P chairs in waitroom
int F;   // F styles of haircut (from 1 to F)

pid_t* barbers;
int barbers_sem, armchairs_sem, clients_sem, waitingroom_shm;
struct client_info* waitingroom;

void clean_trash () {

    for (int i=0; i<M; i++)
        kill(barbers[i], SIGINT);
    free(barbers);

    for (int i=1; i<=P; i++)
        if (waitingroom[i].pid != 0)
            kill((pid_t)waitingroom[i].pid, SIGINT);
    

    if (semctl(barbers_sem, 0, IPC_RMID, NULL) == -1) {
        perror("Error in deleting barbers semaphore");
        _exit(-1);
    }
    if (semctl(armchairs_sem, 0, IPC_RMID, NULL) == -1) {
        perror("Error in deleting armchairs semaphore");
        _exit(-1);
    }
    if (semctl(clients_sem, 0, IPC_RMID, NULL) == -1) {
        perror("Error in deleting clients semaphore");
        _exit(-1);
    }
    if (shmctl(waitingroom_shm, IPC_RMID, NULL) == -1) {
        perror("Error in deleting waitingroom shared memory");
        _exit(-1);
    }
    
    printf("\nMain has ended correctly.\n");
}

void sigint_handler () {
    exit(0);
}

int main (int argc, char** argv) {
    if (argc != 5) {
        fprintf(stderr, "Wrong number of arguments!\n");
        return -1;
    }
    M= atoi(argv[1]);
    N= atoi(argv[2]);
    P= atoi(argv[3]);
    F= atoi(argv[4]);
    if (M < N) {
        fprintf(stderr, "M shouldn't be less than N!\n");
        return -1;
    }

    key_t barber_key, armchair_key, client_key, waitingroom_key;

    // KEYS
    barber_key= get_barber_key();
    armchair_key= get_armchair_key();
    client_key= get_client_key();
    waitingroom_key= get_waitingroom_key();

    semctl(barbers_sem, 0, IPC_RMID, NULL);
    semctl(armchairs_sem, 0, IPC_RMID, NULL);
    semctl(clients_sem, 0, IPC_RMID, NULL);
    shmctl(waitingroom_shm, IPC_RMID, NULL);
    
    // GENERATE SEMAPHORS + SHARED MEMORY
    barbers_sem= get_semaphore(barber_key, "barber", 0);
    armchairs_sem= get_semaphore(armchair_key, "armchairs", 0);
    clients_sem= get_semaphore(client_key, "clients", 0);
    waitingroom_shm= get_shared_memory(waitingroom_key, P + 1, 0);

    atexit(clean_trash);
    signal(SIGINT, sigint_handler);

    // INITIALIZE SEMAPHORS AND SHARED MEMORY
    union semun arg;
    arg.val= M;
    if (semctl(barbers_sem, 0, SETVAL, arg) == -1) {
        perror("Error in setting value in barber semaphore");
        return -1;
    }
    arg.val= N;
    if (semctl(armchairs_sem, 0, SETVAL, arg) == -1) {
        perror("Error in setting value in armchair semaphore");
        return -1;
    }
    arg.val= 0;
    if (semctl(clients_sem, 0, SETVAL, arg) == -1) {
        perror("Error in setting value in clients semaphore");
        return -1;
    }

    waitingroom= shmat(waitingroom_shm, NULL, 0);
    if (waitingroom == (void*)-1) {
        perror("Error in adding shared memory segment to address space");
        return -1;
    }
    waitingroom[0].pid= P;
    waitingroom[0].hairstyle= 1;    // iterator of first client in queue
    for (int i=1; i<=P; i++) {
        waitingroom[i].pid= 0;
        waitingroom[i].hairstyle= 0;
    }


    // MAKING BARBERS
    barbers= malloc(M * sizeof(pid_t));
    char buf[10];
    for (int i=0; i<M; i++) {
        pid_t pid= fork();
        if (pid == 0) {
            barbers[i]= getpid();
            sprintf(buf, "%d", i+1);
            execl("./barber", "./barber", buf, NULL);
            return 0;
        }
    }

    // MAKING CLIENTS
    srand(time(NULL));
    while (1) {
        sleep((rand() % 4) + 1);

        int hairstyle= (rand() % F) + 1;
        pid_t pid= fork();
        if (pid == 0) {
            sprintf(buf, "%d", hairstyle);
            execl("./client", "./client", buf, NULL);
            return 0;
        }
    }

    return 0;
}