struct words {
    char **tab;
    int max_size;
    int size;
};

struct words *makeWords (const int new_size);

void countData (struct words *w, const char *file_name);

char *getValue (struct words *w, const int index);

void delValue (struct words *w, const int index);

void releaseMemory (struct words *w);