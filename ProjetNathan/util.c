#include "util.h"

key_t* getIPCKeys(size_t number)
{
	if(number == 0)
		return NULL;

	key_t* keys;
	if((keys = malloc(number * sizeof(key_t))) == NULL)
		return NULL;

	/* 
	 * The probability that, at least, two keys are the same in an array is quite low :
 	 *			- 0.00000049 for $number$ = 50
 	 *			- 0.00000498 for $number$ = 500
 	 *			- 0.00005028 for $number$ = 5000
 	 * Then, the unicity is guranteed for (not so) small arrays.
	 */
	for(size_t i = 0; i < number; i++)
		keys[i] = rand() % 999999;

	return keys;
}

int initSharedTable(int shm_key, size_t max_value, short**** process_zones)
{
	if(shm_key < 0)
		return -1;

	int shm_id;

	// allocate process_zones array
	(*process_zones) = malloc(NB_PROCESS * sizeof(short**));

	if((*process_zones) == NULL)
		return -1;

	for(int i = 0; i < NB_PROCESS; i++)
	{
		(*process_zones)[i] = malloc(ZONE_SIDE_SIZE * sizeof(short*));
		
		if((*process_zones)[i] == NULL)
			return -1;
		
		for(int j = 0; j < ZONE_SIDE_SIZE; j++)
		{
			(*process_zones)[i][j] = malloc(ZONE_SIDE_SIZE * sizeof(short));
			if((*process_zones)[i][j] == NULL)
				return -1;
		}
	}

	// create shared memory segment
	if((shm_id = shmget(shm_key, sizeof(short) * SIZE_X * SIZE_Y, IPC_CREAT | 0666)) < 0)
		return -1;

	// initilize the shared table with random values
	short* array = shmat(shm_id, NULL, 0);

	if(array == NULL)
	{
		fprintf(stderr, "Shared array initialization error : %s\n", strerror(errno));
		return -1;
	}

	for(size_t i = 0; i < SIZE_Y; i++)
		for(size_t j = 0; j < SIZE_X; j++)
		{
			size_t process_nb = (i / ZONE_SIDE_SIZE) * NB_PROCESS_LINE + (j / ZONE_SIDE_SIZE);

			array[i * SIZE_X + j] = rand() % (max_value + 1);
			(*process_zones)[process_nb][i % ZONE_SIDE_SIZE][j % ZONE_SIDE_SIZE] = array[i * SIZE_X + j];
		}

	// close access to the shared segment
	shmdt(array);
	
	return shm_id;
}

int initSharedLong(int shm_key, long initial_value)
{
	if(shm_key < 0)
		return -1;

	int shm_id;

	// create shared memory segement
	if((shm_id = shmget(shm_key, sizeof(long), IPC_CREAT | 0666)) < 0)
		return -1;

	// initialize shared integer to $initia_value$
	long* data = shmat(shm_id, NULL, 0);

	*data = initial_value;

	shmdt(data);

	return shm_id;
}

int** initSendingQueueIdArrays(int* ids)
{
	// allocate memory for the array
	int** send_ids = malloc(16 * sizeof(int*));

	if(send_ids == NULL)
		return NULL;

	for(int i = 0; i < 16; i++)
	{
		send_ids[i] = malloc(4 * sizeof(int));

		if(send_ids[i] == NULL)
		{
			for(int j = i; j >= 0; j--)
				free(send_ids[j]);
			return NULL;
		}
	}

	// fill the array with ids or -1
	for(int i = 0; i < 16; i++)
	{
		send_ids[i][0] = i % 4 == 3 ? -1 : ids[i + 1]; // right
		send_ids[i][1] = i % 4 == 0 ? -1 : ids[i - 1]; // left
		send_ids[i][2] = i > 11     ? -1 : ids[i + 4]; // bottom
		send_ids[i][3] = i < 4      ? -1 : ids[i - 4]; // top
	}

	return send_ids;
}

int closeIPCs(int* sem_ids, size_t nsem, int* shm_ids, size_t nshm, int* msg_ids, size_t nmsg)
{
	int has_failed = 0;

	// close semaphores
	if(nsem != 0 && sem_ids != NULL)
	{
		for(size_t i = 0; i < nsem; i++)
			if(semctl(sem_ids[i], 0, IPC_RMID) < 0)
			{
				fprintf(stderr, "Cannot close IPC sem. #%zu (id:%d) : %s\n", i, sem_ids[i], strerror(errno));
				has_failed = -1;
			}
	}

	// close shared memory segments
	if(nshm != 0 && shm_ids != NULL)
	{
		for(size_t i = 0; i < nshm; i++)
			if(shmctl(shm_ids[i], IPC_RMID, NULL) < 0)
			{	
				fprintf(stderr, "Cannot close IPC shm. #%zu (id:%d) : %s\n", i, shm_ids[i], strerror(errno));
				has_failed = -1;
			}
	}

	// close message queues
	if(nmsg != 0 && msg_ids != NULL)
	{
		for(size_t i = 0; i < nmsg; i++)
			if(msgctl(msg_ids[i], IPC_RMID, 0) < 0)	
			{
				fprintf(stderr, "Cannot close IPC msg. #%zu (id:%d) : %s\n", i, msg_ids[i], strerror(errno));
				has_failed = -1;
			}
	}

	return has_failed;
}

typedef union semun{
	int val;
	struct semid_ds *buf;
	short *array;
} Semun_t;

void sem_group_signal(int sem_group_id, int value, int start_sem_num, int nb_sems)
{
	for(int i = start_sem_num; i < start_sem_num + nb_sems; i++)
		sem_signal(sem_group_id, value, i);
}

void sem_group_wait(int sem_group_id, int value, int start_sem_num, int nb_sems)
{
	sem_group_signal(sem_group_id, -value, start_sem_num, nb_sems);
}

void sem_group_set(int sem_group_id, int value, int start_sem_num, int nb_sems)
{
	Semun_t set_struct;
	set_struct.array = malloc(nb_sems * sizeof(ushort));

	for(size_t i = 0; i < nb_sems; i++)
		set_struct.array[i] = value;  

	if(semctl(sem_group_id, start_sem_num, SETALL, set_struct))
		fprintf(stderr, "Cannot set value of the group of sem sem. %d : %s\n", sem_group_id, strerror(errno));

	free(set_struct.array);
}

void sem_wait(int sem_id, int value, int num)
{
	sem_signal(sem_id, -value, num);
}

void sem_signal(int sem_id, int value, int num)
{
	struct sembuf signal_struct;

	signal_struct.sem_num = num; // number of the sem to modify in the set
	signal_struct.sem_flg = 0;
	signal_struct.sem_op = value; 

	if(semop(sem_id, &signal_struct, 1) < 0)
		fprintf(stderr, "Cannot wait/signal on sem. %d : %s\n", sem_id, strerror(errno));
}

void sem_set(int sem_id, int value, int num)
{
	Semun_t set_struct;

	set_struct.val = value;

	if(semctl(sem_id, num, SETVAL, set_struct))
		fprintf(stderr, "Cannot set value of sem. %d : %s\n", sem_id, strerror(errno));
}

int sem_get_value(int sem_id, int num)
{
	return semctl(sem_id, num, GETVAL);
}
