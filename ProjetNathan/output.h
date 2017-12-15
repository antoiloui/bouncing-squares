#ifndef _OUTPUT_H_
#define _OUTPUT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <SDL/SDL.h>

#include "constant.h"
#include "shm_manager.h"

int displayer_process(long max_iteration, const int sem_iter_lock_id, 
						const int sem_table_lock_id, const int shm_table_id, 
						const int shm_iter_id, const size_t step);

#endif //_OUTPUT_H_
