master_process(point* segptr,int SQUARE_COUNT,semaphore_Ids) {
	bool allUpdated,isUpdated; 
	//Give access to the shared memory
	signal(access_semid); 
	while(!<ENTER>){
		//Wait before all workers have updated their position
		for(int cntr = 0, cntr < SQUARE_COUNT ; cntr++) 
			wait(posUpdated_semid);

		//Set allUpdated to true in shared memory
		allUpdated = true;     

		//Unlock all semaphores waiting for collision
		for(id = 1; id <= SQUARE_COUNT; id++)
			signal(collision_semid[id-1]);

		updatingPixels();

		//Set isUpdated tag in shared memory for every worker to false 
		for(id = 1; id <= SQUARE_COUNT; id++)
			isUpdated[id] = false;

		//Set allUpdated to false in shared memory
		allUpdated = false;

		//Wait a bit
		usleep(15000); 

			//Unlock all the workers
		for(id = 1; id <= SQUARE_COUNT; id++)
			signal(workers_semid[id-1]);
	}
	wait(control_semid)
}
