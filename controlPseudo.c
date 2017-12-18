control_process(point* segptr, int SQUARE_COUNT,semaphoreIDs,msgq_id,shmid){

    point finish = {.x = 1};
    bool stop = false;
    //Loop until the users wishes to exit
    // by pressing <ENTER>
    while(!stop){
        if(kbhit())
            //Change the loop condition
            stop = true;
            //Set finished to true in shared memory
            finish = true;

            //One last master process interation
            for(int id = 1; id <= SQUARE_COUNT; id++)
                unlocksem(posUpdated_semid,0);
            
            // Wait for the master process to finish its last iteration
            locksem(control_semid,0);       
            // Close messages queues
            remove_queue(msgq_id);
            // Close shared memory
            remove_shm(shmid);
            //Close semaphores
            remove_sem(workers_semid);
            remove_sem(access_semid);
            remove_sem(posUpdated_semid);
            remove_sem(collision_semid);
            remove_sem(control_semid);
    }
}