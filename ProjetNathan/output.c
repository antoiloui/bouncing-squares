#include "output.h"

//Initializes the SDL output
static int init_output(void);

//Updates the SDL output given the content of the state table
static void update_output(short** table);

// free the copy of the state table got from the shared memory 
static void free_table(short** table);

// free surfaces and close SDL
static void close_output(void);

// init the screen
SDL_Surface* screen = NULL;

//Possible colors
SDL_Surface* color_rects[14];

int displayer_process(long max_iteration, const int sem_iter_lock_id, 
					  const int sem_table_lock_id, const int shm_table_id, 
					  const int shm_iter_id, const size_t step)
{
	if(step <= 0)
		return -1;
	// TODO : error messages

	if(max_iteration <= 0)
		max_iteration = MAX_ITERATION;

	if(init_output() < 0) 
		return -1;

	long iteration = 1;

	short** table = NULL;

	while(iteration < max_iteration)
	{
		table = get_shared_table(shm_table_id, sem_table_lock_id);
		if(table == NULL) 
		  	break;

		increment_iteration(shm_iter_id, sem_iter_lock_id, step);
		update_output(table);

		usleep(DISP_SLEEP_TIME * (step < 10 ? step : step/2));
		free_table(table);
		iteration += (long) step;
	}

	close_output();

	return 0;
}


int init_output() 
{
	usleep(100000);
	//Hard-coding the colour values 
	Uint8 colors[14][3];

	colors[0][0] = 0x0000FF; colors[0][1] = 0x000000; colors[0][2] = 0x000000;
	colors[1][0] = 0x0000FF; colors[1][1] = 0x000080; colors[1][2] = 0x000000;
	colors[2][0] = 0x0000FF; colors[2][1] = 0x0000FF; colors[2][2] = 0x000000;
	colors[3][0] = 0x000080; colors[3][1] = 0x0000FF; colors[3][2] = 0x000000;
	colors[4][0] = 0x000000; colors[4][1] = 0x0000FF; colors[4][2] = 0x000000;
	colors[5][0] = 0x000000; colors[5][1] = 0x0000FF; colors[5][2] = 0x000080;
	colors[6][0] = 0x000000; colors[6][1] = 0x0000FF; colors[6][2] = 0x0000FF;
	colors[7][0] = 0x000000; colors[7][1] = 0x000080; colors[7][2] = 0x0000FF;
	colors[8][0] = 0x000000; colors[8][1] = 0x000000; colors[8][2] = 0x0000FF;
	colors[9][0] = 0x00007F; colors[9][1] = 0x000000; colors[9][2] = 0x0000FF;
	colors[10][0] = 0x0000FF; colors[10][1] = 0x000000; colors[10][2] = 0x0000FF;
	colors[11][0] = 0x0000FF; colors[11][1] = 0x000000; colors[11][2] = 0x00007F;
	colors[12][0] = 0x000080; colors[12][1] = 0x000080; colors[12][2] = 0x000080;
	colors[13][0] = 0x000000; colors[13][1] = 0x000000; colors[13][2] = 0x000000;

	//SDL init
	if (SDL_Init(SDL_INIT_VIDEO < 0)) {
		printf("ERROR : Initialisation SDL\n");
		return -1;
	}

	//Setting video mode
	screen = SDL_SetVideoMode(SIZE_Y*PIXEL_WIDTH, SIZE_X*PIXEL_HEIGHT, 32, SDL_SWSURFACE);
	if (screen == NULL) {
		printf("ERROR : Video Mode");
		return -1;
	}

	//Setting title
	SDL_WM_SetCaption("Cyclic cellular automaton (F. Magera & R. Mormont)", NULL);

	// create color rectangle for each state
	for(size_t i = 0; i < 14; i++)
	{
		color_rects[i] = SDL_CreateRGBSurface(SDL_SWSURFACE, PIXEL_WIDTH, PIXEL_HEIGHT, 32, 0, 0, 0, 0);
		Uint32 curr_color = SDL_MapRGB(screen->format, colors[i][0], colors[i][1], colors[i][2]);
		SDL_FillRect(color_rects[i], NULL, curr_color);
	}

	return 0;
}

void update_output(short** table) {
	// create a temporary surface to blit the pixels
	SDL_Surface* tmp_surf = SDL_CreateRGBSurface(SDL_SWSURFACE, screen->w, screen->h, 32, 0, 0, 0, 0);
	SDL_Rect pos, origin;

	origin.x = 0;
	origin.y = 0;

	for(size_t i=0; i < SIZE_Y; ++i)
	{
		pos.y = i * PIXEL_HEIGHT;
		for(size_t j=0; j < SIZE_X; ++j)
			{
				pos.x = j * PIXEL_WIDTH;
				// blit state pixel
				SDL_BlitSurface(color_rects[table[i][j]], NULL, tmp_surf, &pos);
			}

	}
	// blit temporary surface on the screen
	SDL_BlitSurface(tmp_surf, NULL, screen, &origin);
	SDL_FreeSurface(tmp_surf);
	// Makes the update happen on screen
	SDL_Flip(screen);
}

void close_output()
{
	for(size_t i = 0; i < MAX_STATE; i++)
		SDL_FreeSurface(color_rects[i]);

	SDL_Quit();
}

void free_table(short** table)
{
	if(table == NULL)
		return;

  	for(size_t i = 0; i < SIZE_Y; i++)
		free(table[i]);

  	free(table);
}
