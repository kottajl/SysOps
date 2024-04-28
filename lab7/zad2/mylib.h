#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <pwd.h>

#define BARBER_FILE_NAME "/barbers_sem"
#define ARMCHAIR_FILE_NAME "/armchairs_sem"
#define CLIENT_FILE_NAME "/client_sem"
#define WAITINGROOM_FILE_NAME "/waitingroom_shm"

struct client_info {
    int pid;            // <- at 0th element there will be P-value stored there
    int hairstyle;      // <- at 0th element there will be stored iterator of first spot in queue
};

int max (int a, int b) {
    if (a >= b)
        return a;

    return b;
}


int get_val_of_sem (sem_t *sem) {
    int temp;
    if (sem_getvalue(sem, &temp) == -1) {
        perror("Error in reading semaphore value");
        exit(-1);
    }

    return temp;
}


sem_t* get_semaphore (char* name, unsigned int val, char* description, int mode) {
    // mode = 0 -> new semaphore; mode = 1 -> existing semaphore

    sem_t* sem;
    if (mode == 0) {
        sem= sem_open(name, O_CREAT, 0666, val);
        if (sem == SEM_FAILED) {
            fprintf(stderr, "Error in generating %s semaphore ", description);
            perror("");
            exit(-1);
        }
    }
    else {
        sem= sem_open(name, 0);
        if (sem == SEM_FAILED) {
            fprintf(stderr, "Error in opening %s semaphore ", description);
            perror("");
            exit(-1);
        }
    }
    
    return sem;
}


int get_shared_memory (char* name, int mode) {
    // mode = 0 -> new memory; mode = 1 -> existing memory

    int shm;
    if (mode == 0) {
        shm= shm_open(name, O_RDWR | O_CREAT, 0666);
        if (shm == -1) {
            perror("Error in generating waitingroom shared memory segment");
            exit(-1);
        }
    }
    else {
        shm= shm_open(name, O_RDWR, 0);
        if (shm == -1) {
            perror("Error in opening waitingroom shared memory segment");
            exit(-1);
        }
    }

    return shm;
}