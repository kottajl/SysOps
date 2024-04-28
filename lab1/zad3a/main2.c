#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <dlfcn.h>

clock_t start_time, end_time;
struct tms start_cpu, end_cpu;

void startClock () {
    start_time= times(&start_cpu);
}

void endClock (char *info) {
    end_time= times(&end_cpu);

    printf("%s -> ", info);
    printf( "Real time: %lfs, User time: %lfs, System time: %lfs\n",
        (double)(end_time - start_time) / sysconf(_SC_CLK_TCK),
        (double)(end_cpu.tms_utime - start_cpu.tms_utime) / sysconf(_SC_CLK_TCK),
        (double)(end_cpu.tms_stime - start_cpu.tms_stime) / sysconf(_SC_CLK_TCK) );
}


int main () {
    void *handle = dlopen("liblib.so", RTLD_LAZY);
    if (!handle) {
        printf("Blad ladowania biblioteki!\n");
        // printf("%s\n", dlerror());
        return -1;
    }

    struct words* (*makeWords)(const int)= dlsym(handle, "makeWords");
    void (*countData)(struct words*, const char*)= dlsym(handle, "countData");
    char* (*getValue)(struct words*, const int)= dlsym(handle, "getValue");
    void (*delValue)(struct words*, const int)= dlsym(handle, "delValue");
    void (*releaseMemory)(struct words*)= dlsym(handle, "releaseMemory");

    if (dlerror() != NULL) {
        printf("Blad ladowania funkcji biblioteki!\n");
        return -1;
    }


    char input[101];
    struct words *my_words= NULL;
    int my_size= 0;
    int my_maxsize= 0;

    while (fgets(input, 100, stdin)) {
        char *temp;
        temp= strtok(input, " ");

        if (strcmp(temp, "init") == 0) {
            temp= strtok(NULL, " ");
            const int n= atoi(temp);
            if (n < 1 || n > 10000) {
                printf("Niepoprawna liczba!\n");
                continue;
            }
            if (my_words != NULL) {
                printf("Najpierw usun stary bufor!\n");
                continue;
            }

            startClock();
            my_words= makeWords(n);
            my_size= 0;
            my_maxsize= n;
            endClock("init");
        }

        else if (strcmp(temp, "count") == 0) {
            temp= strtok(NULL, " ");

            if (my_words == NULL) {
                printf("Najpierw zainicjalizuj bufor!\n");
                continue;
            }

            if (my_size == my_maxsize) {
                printf("Bufor jest juz zapelniony!\n");
                continue;
            }

            startClock();
            countData(my_words, temp);
            my_size++;
            endClock("count");
        }

        else if (strcmp(temp, "show") == 0) {
            temp= strtok(NULL, " ");

            if (my_words == NULL) {
                printf("Najpierw zainicjalizuj bufor!\n");
                continue;
            }

            const int it= atoi(temp);
            if (it < 0 || it >= my_maxsize) {
                printf("Niepoprawna liczba!\n");
                continue;
            }

            startClock();
            printf("%s\n", getValue(my_words, atoi(temp)));
            endClock("show");
        }

        else if (strcmp(temp, "delete") == 0) {
            temp= strtok(NULL, " ");

            if (my_words == NULL) {
                printf("Najpierw zainicjalizuj bufor!\n");
                continue;
            }

            const int it= atoi(temp);
            if (it < 0 || it >= my_size) {
                printf("Niepoprawna liczba!\n");
                continue;
            }

            startClock();
            delValue(my_words, it);
            my_size--;
            endClock("delete");
        }

        else if (strcmp(temp, "destroy\n") == 0) {
            if (my_words == NULL) {
                printf("Nie ma czego usuwac!\n");
                continue;
            }

            startClock();
            releaseMemory(my_words);
            my_words= NULL;
            my_size= 0;
            my_maxsize= 0;
            endClock("destroy");
        }

        else if (strcmp(temp, "exit\n") == 0)
            break;

        else continue;
    }

    dlclose(handle);
    return 0;
}