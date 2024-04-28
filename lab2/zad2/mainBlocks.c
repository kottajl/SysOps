#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// void reverseSingleBytes (const char *f1name, const char *f2name) {
//     FILE *f1, *f2;
//     f1= fopen(f1name, "r");
//     f2= fopen(f2name, "w");

//     char c;
//     for (int i=1; fseek(f1, -i, SEEK_END) == 0; i++) {      // !=0 => before first character
//         fread(&c, sizeof(char), 1, f1);
//         fwrite(&c, sizeof(char), 1, f2);
//     }

//     fclose(f1);
//     fclose(f2);
// }

void reverse_ (const char *c1, char *c2, const int len) {
    for (int i=0; i<len; i++)
        c2[i]= c1[len - i - 1];
    c2[len]= '\0';
}

void reverseBlocks (const char *f1name, const char *f2name, const int block_size) {
    FILE *f1, *f2;
    f1= fopen(f1name, "r");
    f2= fopen(f2name, "w");

    char c1[block_size + 1];
    char c2[block_size + 1];

    fseek(f1, 0, SEEK_END);
    int len= (int)ftell(f1);
  
    while (len > block_size) {
        fseek(f1, -block_size, SEEK_CUR);

        fread(c1, sizeof(char), block_size, f1);
        reverse_(c1, c2, block_size);
        fwrite(c2, sizeof(char), block_size, f2);

        fseek(f1, -block_size, SEEK_CUR);
        len-= block_size;
    }

    fseek(f1, 0, SEEK_SET);
    fread(c1, sizeof(char), len, f1);
    reverse_(c1, c2, len);
    fwrite(c2, sizeof(char), len, f2);

    fclose(f1);
    fclose(f2);
}


int main (int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Wrong number of arguments passed!\n");
        return -1;
    }

    struct timespec start_time, end_time;

    // clock_gettime(CLOCK_REALTIME, &start_time);
    // reverseSingleBytes(argv[1], argv[2]);
    // clock_gettime(CLOCK_REALTIME, &end_time);
    // printf("r/w on single bytes time: %.2lfms\n", (double)((end_time.tv_nsec - start_time.tv_nsec) / 1000000.0));

    clock_gettime(CLOCK_REALTIME, &start_time);
    reverseBlocks(argv[1], argv[2], 1024);
    clock_gettime(CLOCK_REALTIME, &end_time);
    printf("r/w on blocks of 1024 bytes time: %.2lfms\n", (double)((end_time.tv_nsec - start_time.tv_nsec) / 1000000.0));

    return 0;
}
