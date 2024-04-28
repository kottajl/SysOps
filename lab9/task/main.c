#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>

const int* zero_ptr= 0;
pthread_t reindeers[9];
pthread_t elves[10];
pthread_t st_claus;

pthread_mutex_t reindeer_mutex, reindeer_wait_mutex;
pthread_mutex_t elf_mutex, elf_wait_mutex;
pthread_mutex_t santa_sleep_mutex;

pthread_cond_t santa_sleep_cond= PTHREAD_COND_INITIALIZER;
pthread_cond_t reindeer_wait_cond= PTHREAD_COND_INITIALIZER;
pthread_cond_t elf_wait_cond= PTHREAD_COND_INITIALIZER;

int waiting_elves;
int is_waiting[11];
int waiting_reindeers;
int queue[3];


void init_mutexes () {
    if (pthread_mutex_init(&reindeer_mutex, NULL) == -1) {
        perror("Error in initializing reindeer mutex!\n");
		exit(-1);
    }
    if (pthread_mutex_init(&reindeer_wait_mutex, NULL) == -1) {
        perror("Error in initializing reindeer_wait mutex!\n");
		exit(-1);
    }

    if (pthread_mutex_init(&elf_mutex, NULL) == -1) {
        perror("Error in initializing elf mutex!\n");
		exit(-1);
    }
    if (pthread_mutex_init(&elf_wait_mutex, NULL) == -1) {
        perror("Error in initializing elf_wait mutex!\n");
		exit(-1);
    }

    if (pthread_mutex_init(&santa_sleep_mutex, NULL) == -1) {
        perror("Error in initializing santa mutex!\n");
		exit(-1);
    }
}


void clean_garbage () {
    // threads
    for (int i=0; i<9; i++) {
        if (pthread_cancel(reindeers[i]) != 0) {
			perror("Error in canceling reindeer thread!\n");
			_exit(-1);
		}
    }
    for (int i=0; i<10; i++) {
        if (pthread_cancel(elves[i]) != 0) {
			perror("Error in canceling elf thread!\n");
			_exit(-1);
		}
    }
    if (pthread_cancel(st_claus) != 0) {
		perror("Error in canceling st_claus thread!\n");
		_exit(-1);
	}

    // mutexes
    if (pthread_mutex_destroy(&reindeer_mutex) == -1) {
        perror("Error in destroying reindeer mutex!\n");
		_exit(-1);
    }
    if (pthread_mutex_destroy(&reindeer_wait_mutex) == -1) {
        perror("Error in destroying reindeer_wait mutex!\n");
		_exit(-1);
    }
    if (pthread_mutex_destroy(&elf_mutex) == -1) {
        perror("Error in destroying elf mutex!\n");
		_exit(-1);
    }
    if (pthread_mutex_destroy(&elf_wait_mutex) == -1) {
        perror("Error in destroying elf_wait mutex!\n");
		_exit(-1);
    }
    if (pthread_mutex_destroy(&santa_sleep_mutex) == -1) {
        perror("Error in destroying santa mutex!\n");
		_exit(-1);
    }

    // conditions
    if (pthread_cond_destroy(&reindeer_wait_cond) == -1) {
        perror("Error in destroying reindeer_wait condition");
        _exit(-1);
    }
    if (pthread_cond_destroy(&elf_wait_cond) == -1) {
        perror("Error in destroying elf_wait condition");
        _exit(-1);
    }
    if (pthread_cond_destroy(&santa_sleep_cond) == -1) {
        perror("Error in destroying santa_sleep condition");
        _exit(-1);
    }

}

void sigint_handler () { exit(0); }

void* st_claus_work (void* arg) {
    int journeys= 0;
    int loop_flag= 0;

    while (1) {
        
        pthread_mutex_lock(&santa_sleep_mutex);
        while (waiting_elves < 3 && waiting_reindeers < 9)
            pthread_cond_wait(&santa_sleep_cond, &santa_sleep_mutex);
        pthread_mutex_unlock(&santa_sleep_mutex);

        printf("Mikolaj: budze sie\n");
        fflush(stdout);
        loop_flag= 1;

        while (loop_flag == 1) {
            loop_flag= 0;

            pthread_mutex_lock(&elf_mutex);
            if (waiting_elves == 3) {
                printf("Mikolaj: rozwiazuje problemy elfow %d %d i %d\n", queue[0], queue[1], queue[2]);
                fflush(stdout);
                usleep(1000 * (rand() % 1000 + 1000));

                pthread_mutex_lock(&elf_wait_mutex);
                waiting_elves= 0;
                for (int i=0; i<3; i++) {
                    is_waiting[queue[i]]= 0;
                    queue[i]= 0;
                }
                is_waiting[0]= 0;
                pthread_cond_broadcast(&elf_wait_cond);
                pthread_mutex_unlock(&elf_wait_mutex);
            }
            pthread_mutex_unlock(&elf_mutex);


            pthread_mutex_lock(&reindeer_mutex);
            if (waiting_reindeers == 9) {
                printf("Mikolaj: dostarczam zabawki\n");
                fflush(stdout);
                usleep(1000 * (rand() % 2000 + 2000));

                pthread_mutex_lock(&reindeer_wait_mutex);
                waiting_reindeers= 0;
                pthread_cond_broadcast(&reindeer_wait_cond);
                pthread_mutex_unlock(&reindeer_wait_mutex);

                printf("Mikolaj: jestem po podrozy\n");
                if (++journeys >= 3) {
                    printf("Mikolaj: dobra, fajrant na dzis\n");
                    fflush(stdout);
                    pthread_mutex_unlock(&reindeer_mutex);
                    exit(0);
                }
                loop_flag= 1;
            }
            pthread_mutex_unlock(&reindeer_mutex);

        }
        

        printf("Mikolaj: zasypiam\n");
        fflush(stdout);
    }

    return arg;
}

void* reindeer_work (void* arg) {
    int my_id= (int*)arg - zero_ptr + 1;

    while (1) {

        // after going back to St Claus, w8 before next vacation
        pthread_mutex_lock(&reindeer_wait_mutex);
        while (waiting_reindeers > 0)
            pthread_cond_wait(&reindeer_wait_cond, &reindeer_wait_mutex);
        pthread_mutex_unlock(&reindeer_wait_mutex);

        // vacation
        usleep(1000 * (rand() % 5000 + 5000));

        // going back
        pthread_mutex_lock(&reindeer_mutex);
        waiting_reindeers++;
        printf("Renifer %d: czeka %d reniferow na Mikolaja\n", my_id, waiting_reindeers);

        if (waiting_reindeers == 9) {
            printf("Renifer %d: wybudzam Mikolaja\n", my_id);
            fflush(stdout);
            pthread_mutex_lock(&santa_sleep_mutex);
            pthread_cond_broadcast(&santa_sleep_cond);
            pthread_mutex_unlock(&santa_sleep_mutex);
        }

        pthread_mutex_unlock(&reindeer_mutex);
    }

    return arg;
}


void* elf_work (void* arg) {
    int my_id= (int*)arg - zero_ptr + 1;

    while (1) {

        // if I'm waiting in queue, block here my actions
        pthread_mutex_lock(&elf_wait_mutex);
        if (is_waiting[my_id] == 1) {
            while (waiting_elves > 0)
                pthread_cond_wait(&elf_wait_cond, &elf_wait_mutex);
        }
        pthread_mutex_unlock(&elf_wait_mutex);

        usleep(1000 * (rand() % 3000 + 2000));

        pthread_mutex_lock(&elf_mutex);

        if (waiting_elves >= 3) {
            printf("Elf %d: samodzielnie rozwiazuje swoj problem\n", my_id);
            fflush(stdout);
            pthread_mutex_unlock(&elf_mutex);
        }
        else {
            queue[waiting_elves++]= my_id;
            printf("Elf %d: czeka %d elfow na Mikolaja\n", my_id, waiting_elves);
            fflush(stdout);
            is_waiting[my_id]= 1;

            if (waiting_elves == 3) {
                printf("Elf %d: wybudzam Mikolaja\n", my_id);
                fflush(stdout);
                is_waiting[0]= 1;
                pthread_mutex_lock(&santa_sleep_mutex);
                pthread_cond_broadcast(&santa_sleep_cond);
                pthread_mutex_unlock(&santa_sleep_mutex);
            }
        }

        pthread_mutex_unlock(&elf_mutex);
    }

    return arg;
}


int main () {

    srand(time(NULL));
    waiting_elves= 0;
    waiting_reindeers= 0;
    for (int i=0; i<11; i++)
        is_waiting[i]= 0;

    printf("Poczatek dnia pracy!\n");
    fflush(stdout);
    init_mutexes();

    if (pthread_create(&st_claus, NULL, st_claus_work, NULL) != 0) {
		perror("Error in creating pthread");
		exit(1);
	}

    for (int i=0; i<9; i++)
        if (pthread_create(&reindeers[i], NULL, reindeer_work, (void*)(zero_ptr + i)) != 0) {
			perror("Error in creating pthread");
			exit(1);
		}
    
    for (int i=0; i<10; i++)
        if (pthread_create(&elves[i], NULL, elf_work, (void*)(zero_ptr + i)) != 0) {
			perror("Error in creating pthread");
			exit(1);
		}
    
    signal(SIGINT, sigint_handler);
    atexit(clean_garbage);
    pause();

    return 0;
}