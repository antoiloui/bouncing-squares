#include "shm_manager.h"

// TODO : args check
int update_shared_table(const int shm_id, short** zone, const size_t process_number, const int lock_tab_sem_id)
{
	short* shared_array = shmat(shm_id, (void*) 0, 0);

	if(shared_array == (void*) -1)
		return -1;

	// The line and column at which the process can start to write its zone
	size_t start_i = (process_number / NB_PROCESS_LINE) * ZONE_SIDE_SIZE,
			start_j = (process_number % NB_PROCESS_LINE) * ZONE_SIDE_SIZE;

	sem_wait(lock_tab_sem_id, 1, process_number);

	for(size_t i = 0; i < ZONE_SIDE_SIZE; i++)
		for(size_t j = 0; j < ZONE_SIDE_SIZE; j++)
			shared_array[(i + start_i) * SIZE_X + (j + start_j)] = zone[i][j];

	shmdt(shared_array);

	return 0;
}

short** get_shared_table(const int shm_table_id, const int lock_tab_sem_id)
{
	
	short* shared_array = shmat(shm_table_id, (void*) 0, SHM_RDONLY);

	if(shared_array == (void*) -1)
		return NULL;

	// init the array to be returned
	short** to_return = malloc(SIZE_Y * sizeof(short*));

	if(to_return == NULL)
	{
		shmdt(shared_array);
		return NULL;
	}

	for(int i = 0; i < SIZE_Y; i++)
	{
		to_return[i] = malloc(SIZE_X * sizeof(short));
		if(to_return[i] == NULL)
		{
			for(int j = i - 1; j > 0; j--)
				free(to_return[i]);
			free(to_return);
			shmdt(shared_array);
			return NULL;
		}
	}

	
	// wait for all the worker processes to have written their data
	sem_group_wait(lock_tab_sem_id, 0, 0, NB_PROCESS);
	
	// get the shared table
	for(size_t i = 0; i < SIZE_Y; i++)
	{
		for(size_t j = 0; j < SIZE_X; j++)
			to_return[i][j] = shared_array[i * SIZE_X + j];
	}
	

	// give access to the shared array to the worker processes
	sem_group_set(lock_tab_sem_id, 1, 0, NB_PROCESS);

	shmdt(shared_array);

	return to_return;

}

long get_current_iteration(const int shm_iter_id, const int mutex_sem_id)
{
	long *shared_long = shmat(shm_iter_id, (void*) 0, SHM_RDONLY), 
		to_return = 0;

	if(shared_long == (void*) -1)
		return -1;

	sem_wait(mutex_sem_id, 1, 0);

	to_return = *shared_long; 

	sem_signal(mutex_sem_id, 1, 0);

	shmdt(shared_long);

	return to_return;
}

int increment_iteration(const int shm_iter_id, const int mutex_sem_id, const size_t step)
{
	long *shared_long = shmat(shm_iter_id, (void*) 0, 0);

	if(shared_long == (void*) -1)
		return -1;

	sem_wait(mutex_sem_id, 1, 0);

	*shared_long += step;

	sem_signal(mutex_sem_id, 1, 0);

	shmdt(shared_long);

	return 0;
}