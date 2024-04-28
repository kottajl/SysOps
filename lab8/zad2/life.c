#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include <stdbool.h>
#include "grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>

struct cell {
	int row;
	int col;
};

struct cell_set {
	int n;
	struct cell* tab;
};

int n;
char* foreground;
char* background;
pthread_t* threads_ptr;
struct cell_set* cells_info_ptr;


void handler () {}

void releaseMemory () {

	for (int i=0; i<n; i++) {
		if (pthread_cancel(threads_ptr[i]) != 0) {
			perror("Error in canceling thread!\n");
			_exit(-1);
		}

		free(cells_info_ptr[i].tab);
	}
}

void* function (void* arg) {
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);	// to end immediately when canceled
	struct cell_set* my_cell_set= (struct cell_set*)arg;

	while (true) {
		pause();

		for (int i=0; i<my_cell_set->n; i++)
			update_grid(foreground, background, my_cell_set->tab[i].row, my_cell_set->tab[i].col);
	}

	return my_cell_set;
}


int main(int argc, char** argv) {
	if (argc != 2) {
		fprintf(stderr, "Wrong number of arguments!\n");
		return -1;
	}

	n= atoi(argv[1]);
	if (n <= 0) {
		fprintf(stderr, "Wrong input!\n");
		_exit(1);
	}

	srand(time(NULL));
	setlocale(LC_CTYPE, "");
	initscr(); // Start curses mode

	foreground= create_grid();
	background= create_grid();
	char *tmp;
	init_grid(foreground);

	int cells_per_thread= (GRID_WIDTH * GRID_HEIGHT) / n;
	int additional_cells= (GRID_WIDTH * GRID_HEIGHT) % n;

	pthread_t threads[n];
	threads_ptr= threads;
	struct cell_set cells_info[n];
	signal(SIGUSR1, handler);
	atexit(releaseMemory);
	
	int sum_it= 0;
	for (int i=0; i<n; i++) {

		int k= cells_per_thread;
		if (i < additional_cells)
			k++;
		
		cells_info[i].n= k;
		cells_info[i].tab= malloc(k * sizeof(struct cell));
		
		for (int j=0; j<k; j++) {
			cells_info[i].tab[j].row= (sum_it + j) / GRID_WIDTH;
			cells_info[i].tab[j].col= (sum_it + j) % GRID_WIDTH;
		}
		sum_it+= k;

		if (pthread_create(&threads[i], NULL, function, &cells_info[i]) != 0) {
			perror("Error in creating pthread");
			exit(1);
		}
	}

	while (true) {
		draw_grid(foreground);
		usleep(200 * 1000);

		// Step simulation
		for (int i=0; i<n; i++)
			pthread_kill(threads[i], SIGUSR1);

		usleep(300 * 1000);
		tmp= foreground;
		foreground= background;
		background= tmp;
		
	}

	endwin(); // End curses mode
	destroy_grid(foreground);
	destroy_grid(background);

	return 0;
}
