/******************SHARED MEMORY***********/


square squares_table[SQUARE_COUNT]; //To store position of the squares
semaphore: workers[iterations] = {0,0,0,0,0,0,....}
semaphore : accessPositionTable = 1;
//tableau : position_updated = 0 0 0 0 0 1 0 0
bool allUpdated = true;
int finish = 0;
/*****************************************/
master_process(int iterations) { 

	while(!<ENTER>){

		allUpdated = false;

		for(int j = 0; j < iterations; j++)
			signal(workers[j]) //Call worker 
		signal(access);

		for(all workers){//will only continue when all workers have updated
						// their position
			wait(updatePosition)
		}
		while(all workers not at end of process)
			signal(collision_id)

		display_new_positions

		usleep(15000)

	}
}



process_worker(id) {

	int x, y,speedx,speedy; // 


	while(!<ENTER>){
		// Will wait before calculating it's position until signaled by the master process


		//Need to protect the table with a mutex
		wait(accessPositionTable); // Mutex will be 0, so if another process tries to access, it will be denied
			get_position
			get_speed
			check_next_position

			check_OFB:
				if(OFB_left_right)
					speedx *= -1
				else
					speedy *= -1

			check_collision:
				for(all workers)
					if id is in collision with another worker
						if worker in collision has its position updated (to check )
							signal(collision_id)
							q!(id_collision)// Send vitesse
							q?(my_Id) // Receive vitesse
							speed2 = speed1;
							
					else
						nothing

			store_position

		update_position
		isUpdated;
		signal(accessPositionTable);
		signal(hasUpdated)
		
		wait(collision_id);// Wait for the first collision
		while(!allUpdated){
			q?(id_collision)//Receive vitesse
			speed1 = speed2;
			q!(my_Id)// Send vitesse

			wait(collision_id); // wait for subsequent collisions
		}


			wait(workers[id]);
	}
 

}
/****************************************************************************************************************************
REMARQUES :

	- 



*/