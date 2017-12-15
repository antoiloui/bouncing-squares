#ifndef INIT_H_DEFINED
#define INIT_H_DEFINED

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/sem.h>

#include "constant.h"

/*
 *	FUNCTION getIPCKeys :
 *		Generate a set of integers that can be used as keys to create IPC element (message queue,...).
 *		The array returned must be freed after use.
 *
 * 		PARAMETERS : 
 *			$number$ : number of keys to generate (must be greater than 0).
 *		RETURN :
 * 			an array of type 'long' and size $number$ containing the keys
 *			NULL on error
 */
key_t* getIPCKeys(size_t number);

/*
 * FUNCTION initSharedTable :
 * 		Initialize the shared memory segment for the state table. Then initializes the nodes randomly. 
 *		The key referencing the shared memory segment is $shm_key$.
 *
 *		The initial states to send to each processes are stored in the initially uninitialized $process_zones$
 *		array. This array must be freed after use.
 *		
 *		PARAMETERS :
 *			$shm_key$ : the key to create the shared memory segment
 *			$max_value$ : max value
 *			$process_zones$ : array 16x64x64 that will contain the initial states of each Zone 
 *		
 *		RETURN :
 *			- id of the shared memory segment
 *			- -1 on error
 *		
 *		RETURN BY PARAMETERS :
 *			- $process_zones$ : an array of size (NB_PROCESS x ZONE_SIDE_SIZE x ZONE_SIDE_SIZE)
 *								 which contains the initial states of each Zone 
 */
int initSharedTable(int shm_key, size_t max_value, short**** process_zones);

/*
 *	FUNCTION initSharedLong :
 * 		Initialize a shared memory segment to store an (long) integer which will be set to $initial_value$.
 *
 *		PARAMETERS :
 * 			$shm_key$ : the key to create the shared memory segment
 *			$initial_value$ : the value to set the shared long
 *
 *		RETURN :
 *			- id of the shared memory segment
 *			- -1 on error
 */
int initSharedLong(int shm_key, long initial_value);

/*
 *	FUNCTION initSendingQueueIdArrays : 
 *		The function initializes an array of arrays that contain the ids of the message queue by which 
 *		a given process can send messages to its neighbours. Each line of the array corresponds to a process 
 * 		and each column corresponds to a neighbour. If there is less than 4 neighbours, the array elements 
 *		corresponding to non-existing neighbour are set to -1.
 *
 *		The array returned must be freed after use.
 *	
 *		PARAMETERS :
 *			- ids : array of ids of size 16
 *
 *		RETURN :
 *			- a pointer to the 2D array of size 16 x 4 
 *			- NULL on error 
 */
int** initSendingQueueIdArrays(int* ids);

/*
 *	FUNCTION closeIPCs :
 *		Clear the ipcs base on their ids. The arrays of ids are runned from index 0 to index $nsems/nshm/nmsg$ - 1.
 *
 *		IPC set of sepmaphore are supposed to contain only one semaphore
 *
 *  	PARAMETERS : 
 *			- $sem_ids$ : pointer to an array containing the ids of the IPC semaphore to clear 
 *			- $nsem$ : number of ids in $sem_ids$ 
 *			- $shm_ids$ : pointer to array containing the ids of the IPC shared memory segment to clear
 *			- $nshm$ : number of ids in $shm_ids$
 *			- $msg_ids$ : pointer to array containing the ids of the IPC message queue to clear
 *			- $nmsg$ : number of ids in $msg_ids$
 *	
 *		RETURN :  
 *			- '-1' on error (error = some ipc might not have been closed)	
 *			-  '0' if every given IPC was successfully cleared		
 */
int closeIPCs(int* sem_ids, size_t nsem, int* shm_ids, size_t nshm, int* msg_ids, size_t nmsg);

/*
 *	FUNCTION sem_wait :
 * 		substract the value of the semaphore from a given amount.
 *
 *		PARAMETERS :
 * 			- sem_id : the id of the semaphore
 *			- value : the value we substract to the semaphore
 *			- num : the number of the semaphore in the set created
 *
 *		
 */
void sem_wait(int sem_id, int value, int num);

/*
 *	FUNCTION sem_signal :
 * 		add to the value of the semaphore a given amount.
 *
 *		PARAMETERS :
 * 			- sem_id : the id of the semaphore
 *			- value : the value we add to the semaphore
 *			- num : the number of the semaphore in the set created
 *
 *		
 */
void sem_signal(int sem_id, int value, int num);

/*
 *	FUNCTION sem_set :
 * 		set the value of the semaphore to a given amount.
 *
 *		PARAMETERS :
 * 			- sem_id : the id of the semaphore
 *			- value : the value we add to the semaphore
 *			- num : the number of the semaphore in the set 
 *
 *		
 */
void sem_set(int sem_id, int value, int num);

/*
 *	FUNCTION sem_get_value :
 * 		Returns the value of the semaphore.
 *
 *		PARAMETERS :
 * 			- sem_id : the id of the semaphore
 *			- num : the number of the semaphore in the set 
 *		RETURNS
 *			- the value of the semaphore
 *		
 */
int sem_get_value(int sem_id, int num);

/*
 *	FUNCTION sem_group_signal :
 * 		add to a set of semaphore a value
 *
 *		PARAMETERS :
 * 			- sem_set_id : the id of the semaphores's set
 *			- value : the value we add to the semaphore
 *			- start_sem_num : the number of the first semaphore to change in the set 
 *			- nb_sems: the number of semaphores in the set
 *		
 */
void sem_group_signal(int sem_set_id, int value, int start_sem_num, int nb_sems);

/*
 *	FUNCTION sem_group_signal :
 * 		substract to a set of semaphore a value
 *
 *		PARAMETERS :
 * 			- sem_set_id : the id of the semaphores' set
 *			- value : the value we substract to the semaphore
 *			- start_sem_num : the number of the first semaphore to change in the set 
 *			- nb_sems: the number of semaphores in the set
 *		
 */
void sem_group_wait(int sem_set_id, int value, int start_sem_num, int nb_sems);

/*
 *	FUNCTION sem_group_set :
 * 		set the value of a set of semaphores to a given amount.
 *
 *		PARAMETERS :
 * 			- sem_id : the id of the semaphores' set
 *			- value : the value at which we set to the semaphores
 *			- start_sem_num : the number of the first semaphore to change in the set 
 *			- nb_sems: the number of semaphores in the set
 *
 *		
 */
void sem_group_set(int sem_set_id, int value, int start_sem_num, int nb_sems);

#endif