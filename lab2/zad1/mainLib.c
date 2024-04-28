#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

void libFaR (const char c1, const char c2, const char *f1_name, const char *f2_name) {
    FILE *f1, *f2;
    f1= fopen(f1_name, "r");
    f2= fopen(f2_name, "w");

    char c;
    while (fread(&c, sizeof(char), 1, f1) == 1) {
        if (c == c1)
            fwrite(&c2, sizeof(char), 1, f2);
        else
            fwrite(&c, sizeof(char), 1, f2);
    }

    fclose(f1);
    fclose(f2);
}

// void systemFaR (const char c1, const char c2, const char *f1_name, const char *f2_name) {
//     int f1= open(f1_name, O_RDONLY);
//     int f2= open(f2_name, O_WRONLY | O_CREAT | O_TRUNC);

//     char c;
//     while (read(f1, &c, 1) == 1) {
//         if (c == c1)
//             write(f2, &c2, 1);
//         else
//             write(f2, &c, 1);
//     }

//     close(f1);
//     close(f2);
// }

int main (int argc, char **argv) {
    if (argc != 5) {
        fprintf(stderr, "Wrong number of arguments passed!\n");
        return -1;
    }

    struct timespec start_time, end_time;

    char c1= argv[1][0];
    char c2= argv[2][0];

    clock_gettime(CLOCK_REALTIME, &start_time);
    libFaR(c1, c2, argv[3], argv[4]);
    clock_gettime(CLOCK_REALTIME, &end_time);
    printf("Find&replace using lib functions: %.2lfms\n", (double)((end_time.tv_nsec - start_time.tv_nsec) / 1000000.0));

    // clock_gettime(CLOCK_REALTIME, &start_time);
    // systemFaR(c1, c2, argv[3], argv[4]);
    // clock_gettime(CLOCK_REALTIME, &end_time);
    // printf("Find&replace using system functions: %.2lfms\n", (double)((end_time.tv_nsec - start_time.tv_nsec) / 1000000.0));

    return 0;
}