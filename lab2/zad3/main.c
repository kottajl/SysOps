#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

int main () {
    DIR *dir;
    dir= opendir(".");
    struct dirent *entry;
    struct stat buf;
    long long sum= 0;

    while ((entry= readdir(dir)) != NULL) {
        stat(entry->d_name, &buf);
        
        // if (!S_ISDIR(buf.st_mode))
        //     printf("file\n");
        // else
        //     printf("dir\n");

        if (!S_ISDIR(buf.st_mode)) {
            printf("%ld %s\n", buf.st_size, entry->d_name);
            sum+= (long long)buf.st_size;
        } 
    }
    printf("Summary size: %lld bytes\n", sum);
    
    closedir(dir);
    return 0;
}