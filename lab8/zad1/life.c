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

struct cell {
	int row;
	int col;
};

char* foreground;
char* background;
pthread_t threads[GRID_WIDTH * GRID_HEIGHT];

void handler () {}

void* function (void* arg) {
	struct cell* my_cell= (struct cell*)arg;

	while (true) {
		pause();

		update_grid(foreground, background, my_cell->row, my_cell->col);
	}

	return my_cell;
}


int main() {
	srand(time(NULL));
	setlocale(LC_CTYPE, "");
	initscr(); // Start curses mode

	foreground= create_grid();
	background= create_grid();
	char *tmp;

	init_grid(foreground);

	
	struct cell cells_info[GRID_WIDTH * GRID_HEIGHT];
	signal(SIGUSR1, handler);


	for (int i=0; i<GRID_HEIGHT; i++)
		for (int j=0; j<GRID_WIDTH; j++) {
			cells_info[i * GRID_WIDTH + j].row= i;
			cells_info[i * GRID_WIDTH + j].col= j;
			if (pthread_create(&threads[i * GRID_WIDTH + j], NULL, function, &cells_info[i * GRID_WIDTH + j]) != 0) {
				perror("Error in creating pthread");
				exit(1);
			}
		}

	while (true) {
		draw_grid(foreground);
		usleep(500 * 1000);

		// Step simulation
		for (int i=0; i<GRID_HEIGHT; i++)
			for (int j=0; j<GRID_WIDTH; j++)
				pthread_kill(threads[i * GRID_WIDTH + j], SIGUSR1);

		tmp = foreground;
		foreground = background;
		background = tmp;
	}

	endwin(); // End curses mode
	destroy_grid(foreground);
	destroy_grid(background);

	return 0;
}
