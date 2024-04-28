#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <pwd.h>

struct client_info {
    int pid;            // <- at 0th element there will be P-value stored there
    int hairstyle;      // <- at 0th element there will be stored iterator of first spot in queue
};

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};


int max (int a, int b) {
    if (a >= b)
        return a;

    return b;
}

char* getHomePath () {
    char* home_path= getenv("HOME");

    if (home_path == NULL)
        home_path= getpwuid(getuid())->pw_dir;

    return home_path;
}

key_t get_barber_key () {
    key_t key= ftok(getHomePath(), 'b');
    if (key == -1) {
        perror("Error in generating barber key");
        exit(-1);
    }

    return key;
}

key_t get_armchair_key () {
    key_t key= ftok(getHomePath(), 'a');
    if (key == -1) {
        perror("Error in generating armchair key");
        exit(-1);
    }
    
    return key;
}

key_t get_client_key () {
    key_t key= ftok(getHomePath(), 'c');
    if (key == -1) {
        perror("Error in generating client key");
        exit(-1);
    }
    
    return key;
}

key_t get_waitingroom_key () {
    key_t key= ftok(getHomePath(), 'w');
    if (key == -1) {
        perror("Error in generating waitingroom key");
        exit(-1);
    }
    
    return key;
}

int get_semaphore (key_t key, char* description, int mode) {
    // mode = 0 -> new semaphore; mode = 1 -> existing semaphore

    int sem;
    if (mode == 0) {
        sem= semget(key, 1, IPC_CREAT | 0666);
        if (sem == -1) {
            fprintf(stderr, "Error in generating %s semaphore ", description);
            perror("");
            exit(-1);
        }
    }
    else {
        sem= semget(key, 0, 0);
        if (sem == -1) {
            fprintf(stderr, "Error in opening %s semaphore ", description);
            perror("");
            exit(-1);
        }
    }
    
    return sem;
}

int get_shared_memory (key_t key, int no_elements, int mode) {
    // mode = 0 -> new memory; mode = 1 -> existing memory

    int shm;
    if (mode == 0) {
        shm= shmget(key, no_elements * sizeof(struct client_info), IPC_CREAT | 0600);
        if (shm == -1) {
            perror("Error in generating waitingroom shared memory segment");
            exit(-1);
        }
    }
    else {
        shm= shmget(key, 0, 0);
        if (shm == -1) {
            perror("Error in opening waitingroom shared memory segment");
            exit(-1);
        }
    }

    return shm;
}