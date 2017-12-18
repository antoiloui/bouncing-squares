/******************SHARED MEMORY***********/
point finish;
point square_position[SQUARE_COUNT]; //To store position of the squares
point isUpdated[SQUARE_COUNT];
point allUpdated;
/******************SEMAPHORE SETS***********/
workers_semid[SQUARE_COUNT] : {0,0,...,0} 
posUpdated_semid[1] = 0; 
access_semid[1] = 0;
collision_semid[SQUARE_COUNT] : {0,0,...,0} 
/******************MESSAGE QUEUES***********/
msgq_id[1];
struct mymsgbuf {
	long type; 
	int sender; 
struct speed_s speed;
// with struct speed_s = {speed_x,speed_y}
};









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
}



worker(int id,point* segptr,int SQUARE_COUNT,semaphore_Ids,int msgq_id,int speed) {
	point next_pos;
	point current_pos;

	while(!<ENTER>){
		//Mutex for the shared memory
		wait(accessPositionTable) 

		getCurrentPosition();
		ComputeNextPosition();
		CheckOutOfBounds();

		//checkCollisionSender
		for(int other_id = 1; other_id <= SQUARE_COUNT; other_id++){
			if(hasIntersection(id,other_id))
				if position_updated[other_id] == true
					signal(collision_semid[other_id-1])
					// Send my speed
					msgq_id!(other_id,mySpeed)
					// Receive other speed
					msgq_id?(id,otherSpeed) 
					updateSpeed();
					updatePosition();
					other_id = 0;
					continue;
			}

				store_position()
		isUpdated[id] = true;  //in shared memory
		//Give access to the shared memory to someone else
		signal(accessPositionTable); 
		//Signal to the master that you are updated
		signal(posUpdated_semid); 

		//checkCollisionReceiver

		// Wait for the first collision
		wait(collision_semid[id-1])
		while(!allUpdated)
			//Receive other speed
			msgq_id?(id,otherSpeed)
			// Send my speed
			msgq_id!(other_id,mySpeed)
			mySpeed = otherSpeed;
			// wait for subsequent collisions
			wait(collision_semid[id-1]); 

		// Wait for the master process	
		wait(workers_semid[id-1]);	
	}
}	
