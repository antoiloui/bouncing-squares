#include <stdio.h> // fprintf, printf
#include <stdlib.h> // size_t
#include <sys/ipc.h> 
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h> 
#include <unistd.h> // getppid
#include <time.h> // time
#include <errno.h> // errno
#include <string.h> // strerror
#include <signal.h> // signal

#include "util.h"
#include "output.h"
#include "automaton.h"
#include "constant.h"
#include "shm_manager.h"

#include "List.h"

// declaring the number of each kind of IPCs that'll be used
#define NSEMS 3
#define NSHMS 2
#define NMSGS 16


// Handler that will be launched when the main process receive a SIGINT or a SIGTERM signal
static void error_handler(int signum);

int shm_ids[NSHMS], sem_ids[NSEMS], msg_ids[NMSGS];
int pid[NB_PROCESS + 1];

void error_handler(int signum)
{
	fprintf(stderr, "Program exited in an unexpected way. You should make sure that : \n");
	fprintf(stderr, "\t - all IPCs were closed\n");
	fprintf(stderr, "\t - all processes were killed\n");
	fprintf(stderr, "even though they should have been closed and killed\n");

	// close IPCs
	closeIPCs(sem_ids, NSEMS, shm_ids, NSHMS, msg_ids, NMSGS);

	// kill child processes
	for(size_t i = 0; i < NB_PROCESS + 1; i++)
		kill(pid[i], SIGKILL);

	exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
	if(argc > 3)
	{
		fprintf(stderr, "USAGE : %s [<max_iteration> [<step>]]\n", argv[0]);
		return EXIT_FAILURE;
	}

	long max_it = -1; // this value will leave the choice of the max iteration to the processes
	size_t step = 1;

	if(argc == 2 || argc == 3)
		max_it = strtoul(argv[1], NULL, 10);
	
	if(argc == 3)
		step = strtoul(argv[2], NULL, 10);

	srand(time(NULL));

	// generate keys for shared memory, semaphore and message queue
	// - 2 shm : one for shared table and one for shared variable 
	// - 3 sem : one for couning process that have written their datas, 
	//				one for mutex on data in shared memory
	//				one for making the main process wait the others
	// - 16 msg
	// => 21 keys
	 // number of IPCs
	key_t *shm_keys = getIPCKeys(NSHMS),
		*sem_keys = getIPCKeys(NSEMS),
		*msg_keys = getIPCKeys(NMSGS);

	// open shared memory and initialize it
	// - shm_ids[0] : array 256x256
	// - shm_ids[1] : iteration  
	short*** processes_init_zone = NULL;

	if((shm_ids[0] = initSharedTable(shm_keys[0], MAX_STATE - 1, &processes_init_zone)) < 0)
	{
		fprintf(stderr, "Cannot initialize shared table : %s\n", strerror(errno));
		return EXIT_FAILURE;
	}
	
	if((shm_ids[1] = initSharedLong(shm_keys[1], 0)) < 0) // init the level counter to 0
	{
		fprintf(stderr, "Cannot initialize shared integer : %s\n", strerror(errno));
		closeIPCs(NULL, 0, shm_ids, 1, NULL, 0);
		return EXIT_FAILURE;
	}

	// open semaphore and initialize it
	// - sem_ids[0] : for locking the automaton's table
	// - sem_ids[1] : for locking the shared variable $iteration$
	// - sem_ids[2] : for locking the main process while the others are running
	if((sem_ids[0] = semget(sem_keys[0], 16, IPC_CREAT | 0666)) < 0)
	{
		perror("Processes sem");
		closeIPCs(NULL, 0, shm_ids, NSHMS, NULL, 0);
		return EXIT_FAILURE;
	}

	if((sem_ids[1] = semget(sem_keys[1], 1, IPC_CREAT | 0666)) < 0)
	{
		perror("Iteration sem");
		closeIPCs(sem_ids, 1, shm_ids, NSHMS, NULL, 0);

		return EXIT_FAILURE;
	}

	if((sem_ids[2] = semget(sem_keys[2], 1, IPC_CREAT | 0666)) < 0)
	{
		perror("Main process sem");
		closeIPCs(sem_ids, 2, shm_ids, NSHMS, NULL, 0);

		return EXIT_FAILURE;
	}

	// set semaphores values
	sem_group_set(sem_ids[0], 1, 0, NB_PROCESS);
	sem_set(sem_ids[1], 1, 0);
	sem_set(sem_ids[2], 0, 0);

	// initilize message queues
	for(size_t i = 0; i < NMSGS; ++i)
	{
		if((msg_ids[i] = msgget(msg_keys[i], IPC_CREAT | 0666)) < 0)
		{
			fprintf(stderr, "Cannot create message queue %zu : %s\n", i, strerror(errno));
			closeIPCs(sem_ids, NSEMS, shm_ids, NSHMS, msg_ids, i);
			return EXIT_FAILURE;
		}
	}

	free(shm_keys); free(sem_keys); free(msg_keys);

	// initialize array containing the send queue ids
	int** send_ids; 

	if((send_ids = initSendingQueueIdArrays(msg_ids)) == NULL)
	{
		fprintf(stderr, "Cannot initialize sending queues' id array\n");
		closeIPCs(sem_ids, NSEMS, shm_ids, NSHMS, msg_ids, NMSGS);
		return EXIT_FAILURE; 
	}

	// initialize 'pid' array value
	for(size_t i = 0; i < NB_PROCESS + 1; i++)
		pid[i] = 0;

	// associate the error_handler to the SIGTERM
	for(int i = 0; i < NB_PROCESS + 1; i++) 
	{ 
		pid[i] = fork(); 
		if(!pid[i])
		{ 
			if(i != 16) // launch worker processes
			{	
				if(automaton_process(i, processes_init_zone[i],send_ids[i], msg_ids[i], sem_ids[0], 
								sem_ids[1], shm_ids[0], shm_ids[1], max_it, step) < 0) // checking error in automaton
					kill(getppid(), SIGTERM); // signals error to the main process
			}
			else  // launch displayer processes
			{
				if(displayer_process(max_it, sem_ids[1], sem_ids[0], shm_ids[0], shm_ids[1], step) < 0)
					kill(getppid(), SIGTERM); // signals error to the main process
			}
			sem_signal(sem_ids[2], 1, 0); 
			break;
		} 
	}

	if(pid[16]) // if in father process
	{
		signal(SIGINT, &error_handler);
		signal(SIGTERM, &error_handler);
		sem_wait(sem_ids[2], NB_PROCESS + 1, 0);
		closeIPCs(sem_ids, NSEMS, shm_ids, NSHMS, msg_ids, NMSGS);
	}
	
	return 0;
}