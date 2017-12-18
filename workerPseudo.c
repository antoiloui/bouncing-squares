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
					if(mySpeed == otherSpeed);
						mySpeed *= -1;
					else 
						mySpeed = otherSpeed;
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
 