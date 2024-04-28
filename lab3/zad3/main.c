#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>

void checkFile (const char *file_path, const char *templ) {
    FILE *f;
    f= fopen(file_path, "r");
    if (f == NULL) {
        perror("Error in opening file to check");
        return;
    }

    errno= 0;
    char input[256];
    size_t inp_len= (int)fread(input, sizeof(char), strlen(templ), f);
    if (errno != 0) {
        perror("Error in reading file to check");
        fclose(f);
        return;
    }

    input[inp_len]= '\0';

    if (strcmp(templ, input) == 0) {
        printf("%s %d\n", file_path, (int)getpid());
        fflush(stdout);
    }
    
    fclose(f);
}

void checkDir (const char *my_dir, const char *templ) {
    errno= 0;
    pid_t pid= fork();

    if (pid != 0 && errno != 0) {
        perror("Error in forking");
        return;
    }

    if (pid == 0) {
        errno= 0;
        DIR *d;
        d= opendir(my_dir);
        if (errno != 0) {
            perror("Error in opening directory");
            exit(0);
        }

        struct dirent *entry;

        errno= 0;
        while ((entry= readdir(d)) != NULL) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            char *temp= malloc(sizeof(char) * PATH_MAX);
            strcpy(temp, my_dir);
            strcat(temp, "/");
            strcat(temp, entry->d_name);

            errno= 0;
            struct stat buf;
            stat(temp, &buf);
            if (errno != 0) {
                perror("Error in reading stats of file/directory");     // (wiem, ze katalog to tez plik w post-unixowych,
                free(temp);                                             //  ale taki blad bedzie czytelniejszy)
                continue;
            }

            if (!S_ISDIR(buf.st_mode))
                checkFile(temp, templ);
            else
                checkDir(temp, templ);
            
            free(temp);
            errno= 0;
        }

        if (errno != 0)
            perror("Error in reading directory");

        closedir(d);
        exit(0);
    }

    errno= 0;
    wait(NULL);
    if (errno != 0)
        perror("Error in waiting for processes");

    return;
}

int main (int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "Wrong number of arguments!\n");
        return -1;
    }

    char templ[256];
    strcpy(templ, argv[2]);

    checkDir(argv[1], templ);

    return 0;
}