#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <ftw.h>

long long sum;

int func (const char *file_path, const struct stat *sb, int typeflag) {
    if (!S_ISDIR(sb->st_mode)) {
        printf("%ld %s\n", sb->st_size, file_path);
        sum+= (long long)sb->st_size;
    }

    // if (typeflag == FTW_F) {
    //     printf("%ld %s\n", sb->st_size, file_path);
    //     sum+= (long long)sb->st_size;
    // }

    return 0;
}


int main (int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Wrong number of arguments!\n");
        return -1;
    }
    sum= 0;

    if (ftw(argv[1], func, 100) == -1) {
        fprintf(stderr, "Error in function ftw!\n");
        return -1;
    }

    printf("Summary size: %lld bytes\n", sum);
    return 0;
}