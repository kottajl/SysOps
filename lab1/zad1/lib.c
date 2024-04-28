#include "lib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct words *makeWords (const int new_size) {
    struct words *new_words= malloc(sizeof(struct words));
    char **new_tab= calloc(new_size, sizeof(char*));

    new_words->tab= new_tab;
    new_words->size= 0;
    new_words->max_size= new_size;

    return new_words;
}


void countData (struct words *w, const char *file_name) {
    char comm[101]= "wc ";
    strcat(comm, file_name);
    comm[strlen(comm)-1]= '\0';
    strcat(comm, " > /tmp/data");
    if (system(comm) == -1)
        return;

    FILE *temp_file;

    temp_file= fopen("/tmp/data", "r");
    if (temp_file == NULL) {
        printf("Blad przy wczytywaniu pliku tymczasowego!\n");
        return;
    }

    char data[101];
    fgets(data, 100, temp_file);
    fclose(temp_file);
    int len= strlen(data);

    char *new_block= calloc(len + 1, sizeof(char));
    strcpy(new_block, data);

    w->tab[w->size]= new_block;
    w->size++;

    remove("/tmp/data");
}


char *getValue (struct words *w, const int index) {
    return w->tab[index];
}

void delValue (struct words *w, const int index) {
    free(w->tab[index]);
    if (index + 1 < w->max_size)
        memmove( w->tab + index, w->tab + index + 1, sizeof(char*) * (w->size - (index + 1)) );
    w->tab[w->size - 1]= NULL;
    w->size--;
}

void releaseMemory (struct words *w) {
    for (int i=0; i < w->max_size; i++)
        free(w->tab[i]);

    free(w->tab);
}