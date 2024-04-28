#include "mylib.h"

int M;   // M barbers
int N;   // N armchairs
int P;   // P chairs in waitroom
int F;   // F styles of haircut (from 1 to F)

pid_t* barbers;
sem_t *barbers_sem, *armchairs_sem, *clients_sem;
int waitingroom_shm;
struct client_info* waitingroom;

void clean_trash () {

    for (int i=0; i<M; i++)
        kill(barbers[i], SIGINT);
    free(barbers);

    for (int i=1; i<=P; i++)
        if (waitingroom[i].pid != 0)
            kill((pid_t)waitingroom[i].pid, SIGINT);
    

    if (sem_close(barbers_sem) == -1) {
        perror("main: Error in closing barbers semaphore");
        _exit(-1);
    }
    if (sem_close(armchairs_sem) == -1) {
        perror("main: Error in closing armchairs semaphore");
        _exit(-1);
    }
    if (sem_close(clients_sem) == -1) {
        perror("main: Error in closing clients semaphore");
        _exit(-1);
    }
    if (munmap(waitingroom, (P+1) * sizeof(struct client_info)) == -1) {
        perror("main: Error in closing waiting room shared memory segment");
        _exit(-1);
    }

    // usleep(1000 * 500);

    if (sem_unlink(BARBER_FILE_NAME) == -1) {
        perror("Error in deleting barbers semaphore");
        _exit(-1);
    }
    if (sem_unlink(ARMCHAIR_FILE_NAME) == -1) {
        perror("Error in deleting armchairs semaphore");
        _exit(-1);
    }
    if (sem_unlink(CLIENT_FILE_NAME) == -1) {
        perror("Error in deleting clients semaphore");
        _exit(-1);
    }
    sem_unlink(WAITINGROOM_FILE_NAME);
    
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

    // sem_unlink(BARBER_FILE_NAME);
    // sem_unlink(ARMCHAIR_FILE_NAME);
    // sem_unlink(CLIENT_FILE_NAME);
    // sem_unlink(WAITINGROOM_FILE_NAME);
    
    // GENERATE SEMAPHORS + SHARED MEMORY
    barbers_sem= get_semaphore(BARBER_FILE_NAME, M, "barbers", 0);
    armchairs_sem= get_semaphore(ARMCHAIR_FILE_NAME, N, "armchairs", 0);
    clients_sem= get_semaphore(CLIENT_FILE_NAME, 0, "clients", 0);
    waitingroom_shm= get_shared_memory(WAITINGROOM_FILE_NAME, 0);
    if (ftruncate(waitingroom_shm, (off_t)((P+1) * sizeof(struct client_info))) == -1) {
        perror("Error in setting size of shared memory");
        return -1;
    }

    atexit(clean_trash);
    signal(SIGINT, sigint_handler);

    // GET ADDRESS OF SHARED MEMORY AND INITIALIZE IT
    waitingroom= mmap(NULL, (P+1) * sizeof(struct client_info), PROT_WRITE | PROT_READ, MAP_SHARED, waitingroom_shm, 0);
    if (waitingroom == (void*)-1) {
        perror("main: Error in adding shared memory segment to address space");
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
    char buf1[10], buf2[10];
    sprintf(buf2, "%d", P);

    for (int i=0; i<M; i++) {
        pid_t pid= fork();
        if (pid == 0) {
            barbers[i]= getpid();
            sprintf(buf1, "%d", i+1);
            execl("./barber", "./barber", buf1, buf2, NULL);
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
            sprintf(buf1, "%d", hairstyle);
            execl("./client", "./client", buf1, buf2, NULL);
            return 0;
        }
    }

    return 0;
}