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
control_semid[1] : 0
/******************MESSAGE QUEUES***********/
msgq_id[1];
struct mymsgbuf {
	long type; 
	int sender; 
struct speed_s speed;
};

struct speed_s{
  int speed_x;
  int speed_y;
};
