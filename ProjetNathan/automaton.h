#ifndef STATE_H_DEFINED
#define STATE_H_DEFINED

#include <stddef.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "shm_manager.h"
#include "constant.h"
#include "util.h"

#include "List.h"
/*
 * FUNCTION automaton_process :
 *		This function coordinates the actions the worker process has to do. 
 *
 *		The number of iteration is specified by the parameter $max_iteration$. A negative value or zero
 *		lead to a max number of iteration specified in 'constant.h'.
 *
 *		PARAMETERS :
 *			- process_number : the process number
 *			- **init_state : the initial state of the nodes of the Zone
 *			- *send_qids : the id of the queues of the neighbours
 *			- receive_qid: the id of the queue of the process
 *			- sem_table_id: the id of the semaphore of the shared table in memory
 * 			- sem_iter_id : the id of the semaphore of the shared variable in memory
 *			- shm_table_id : id of the shared memory segment containing the table
 *			- shm_iter_id : id of the shared memory segment containing the iteration variable
 *			- max_iteration : The maximal number of cycles to do 
 *			- step : the number of iterations to pass (must be greater than 0)
 *			
 *		RETURNS
 *			- -1 if an error occurred 
 *			- 0 otherwise
 */
int automaton_process(size_t process_number, short** init_state, int* send_qids, int receive_qid, 
				int sem_table_id, int sem_iter_id, int shm_table_id,
				int shm_iter_id, long max_iteration, size_t step);

#endif // STATE_H_DEFINED