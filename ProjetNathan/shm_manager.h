#ifndef SHM_MANAGER_H_DEFINED
#define SHM_MANAGER_H_DEFINED

#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#include "util.h"
#include "constant.h" // contains the size of the array

/*
 * FUNCTION update_shared_table :
 *		This function updates a zone of the shared table. The size if the table is defined in 'constant.h'.
 *		This action is performed if the semaphore, whose id is $lock_tab_sem_id$, has a value greater than
 *		0. Otherwise the process executing the function waits for the sem to be greater than 0.
 *
 *		If the update operation worked, the semaphore is decremented.
 *
 *		PARAMETERS :
 *			- $shm_id$ : id of the shared memory segment containing the table
 *			- $zone$ : the zone to copy in the table
 *			- $process_number$ : number of the process updating the table
 *			- $lock_tab_sem_id$ : id of the semaphore set locking the table
 *
 *		RETURNS :
 *			- '-1' on error
 *			- 0 otherwise
 */
int update_shared_table(const int shm_id, short** zone, const size_t process_number, const int lock_tab_sem_id);

/*
 *	FUNCTION get_shared_table :
 *		This function returns the shared table if every semaphore in the set of sem. (identified by 
 *		the id $lock_tab_sem_id$) has value 0.
 *		Otherwise, the calling process waits for the all the semaphores to be 0. 
 *
 *		If the reading operation has succeed, every semaphore in the set is set to 1 so that the 
 *		worker processes can write the next iteration in the shared table.
 *
 *		PARAMETERS :
 *			- $shm_table_id$ : id of the shared memory segment containing the adress
 *			- $lock_tab_sem_id$ : id of the semaphore locking the table
 *		
 *		RETURNS :
 *			- a pointer to the table (must be freed after use) (size of the table defined in 'constant.h')
 *			- NULL on error
 */
short** get_shared_table(const int shm_table_id, const int lock_tab_sem_id);

/*
 *	FUNCTION get_current_iteration :
 *		This function returns the iteration of the table being built in the shared memory. 
 *
 *		If the ressource (shared variable) is used (semaphore has value 0), the calling process wait till the ressource is freed
 *		before reading the value and returning it.
 *
 *		PARAMETERS :
 *			- $shm_iter_id$ : the id of the shared memory segment containing the iteration 
 *			- $mutex_sem_id$ : the id of the semaphore locking the shared variable
 *
 *		RETURNS :
 *			- the current iteration being built in the shared memory
 *			- '-1' on error
 */
long get_current_iteration(const int shm_iter_id, const int mutex_sem_id);

/*
 *	FUNCTION increment_iteration :
 *		This function increments of $step$ the shared variable 'iteration' 
 *
 *		If the ressource (shared variable) is used (semaphore has value 0), the calling process wait till the ressource is freed
 *		before incrementing the value.
 *
 *		PARAMETERS :
 *			- $shm_iter_id$ : the id of the shared memory segment containing the iteration 
 *			- $mutex_sem_id$ : the id of the semaphore locking the shared variable
 *			- $step$ : the increment value
 *
 *		RETURNS :
 *			- 0 if the shared variable was successfully incremented
 *			- '-1' on error
 */
int increment_iteration(const int shm_iter_id, const int mutex_sem_id, const size_t step);

#endif