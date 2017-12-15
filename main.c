#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include<stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "process_communication.h"
#include "constants.h"
#include "output.h"

#define NPROCESS 3

/************************************PROTOTYPES****************************************************/
int hasIntersection(square a, square b);  //Returns 1 if the two squares intersect and 0 otherwise
void initializeSquares(square* squares_table,int SQUARE_COUNT);


/*****************************************PROCESSES**********************************************/


control_process(point* segptr, int SQUARE_COUNT, int workers_semid, int access_semid, int posUpdated_semid, pid_t pid){
  char c;
  point finish;

  while(c = getChar()){
    if(putchar(c) == '\n')
      printf("Enter pressed, program exited.");
      finish.x = 1;
      writeshm(segptr,0,finish);
  }

  //Close semaphores
    removeshm(workers_semid);
    removeshm(access_semid);
    removeshm(posUpdated_semid);

  // Close shared memory segments
    for(size_t i=0; i < SQUARE_COUNT+1; i++){
      removeshm(segptr[i]);
    }
  

  // Close messages queues
  

  // Kill the child processes
  for(size_t i=0; i < NPROCESS+1 ;i++){
    kill(pid[i], SIGKILL);
  }
  


master_process(point* segptr,int SQUARE_COUNT, int* workers_semid, int access_semid, int posUpdated_semid) {

    int table_of_pixels[SIZE_X][SIZE_Y];  //Will store the states of the pixels


  int id,j,k;
  
  //As long as the user doesn't quit
  while(readshm(segptr,0).x != 1) {
    
      //Display
      printf("\nEnter next cycle\n");
      printf("Compute next table\n");

    for(int id = 1; id <= SQUARE_COUNT; id++){
      unlocksem(workers_semid,id);
    }

    int finish = 0;
    int id,j,k;

    //As long as the user doesn't quit
    while((finish = readshm(segptr,0).x) != 1) {
        
        //Display
        printf("\nEnter next cycle\n");
        printf("Compute next table\n");

        for(id = 1; id <= SQUARE_COUNT; id++){
            printf("Process %d updated its position",id);
            unlocksem(workers_semid,id);
        }
>>>>>>> 68b1487ef788872ffc32e80ad2e3dcec30738fe6

        unlocksem(access_semid,0); //Give access to the square table


        //Wait before all workers have updated their position
        for(int cntr = 0; cntr < SQUARE_COUNT ; cntr++) {
            locksem(posUpdated_semid,0);
        }

        //Updating the table_of_pixels
        for(j = 0; j < SIZE_X; j++){
            for(k = 0; k < SIZE_Y; k++){
                table_of_pixels[j][k] = 0;
            }
        }

        for(id = 1; id <= SQUARE_COUNT; id++){
            for(j = 0; j < SQUARE_WIDTH; j++){
                for(k = 0; k < SQUARE_WIDTH; k++){
                    point position = readshm(segptr,id);
                    table_of_pixels[position.x+j][position.y+k] = id % 4;
                }
            }
        }

        //Apply the change on SDL display
        update_output(table_of_pixels);
        //Wait a bit
        usleep(15000);
        
    }
}



worker(int id, int SQUARE_COUNT, point* segptr, int workers_semid, int access_semid, int posUpdated_semid, int speedx, int speedy){

    point next_pos;
    point current_pos;

<<<<<<< HEAD
  while(readshm(segptr,0).x != 1) {

    locksem(access_semid,0); //wait(accessPositionTable)
    //Get current position
    current_pos = readshm(segptr,id);
    //Compute next position
    next_pos.x = current_pos.x + speedx;
    next_pos.y = current_pos.y + speedy;
    //Update position
    writeshm(segptr,id,next_pos);
    unlocksem(access_semid,0);//signal(accessPositionTable)

    unlocksem(posUpdated_semid,0); //has updated it's position
    locksem(workers_semid,id); // Wait for the master process

  }
=======
    while((finish = readshm(segptr,0).x) != 1) {
        printf("Worker %d is working", id);
        locksem(access_semid,0); //wait(accessPositionTable)
        //Get current position
        current_pos = readshm(segptr,id);
        //Compute next position
        next_pos.x = current_pos.x + speedx;
        next_pos.y = current_pos.y + speedy;
        //Update position
        writeshm(segptr,id,next_pos);
        unlocksem(access_semid,0);//signal(accessPositionTable)

        unlocksem(posUpdated_semid,0); //has updated it's position
        locksem(workers_semid,id); // Wait for the master process
    }
>>>>>>> 68b1487ef788872ffc32e80ad2e3dcec30738fe6
}



/******************************************FUNCTIONS***************************************************/

//Do two squares have an intersection?
int hasIntersection(square a, square b){
    int rc = 0;
  
    if(a.y < b.y+SQUARE_WIDTH && a.y+SQUARE_WIDTH > b.y &&
        a.x < b.x+SQUARE_WIDTH && a.x+SQUARE_WIDTH > b.x)
        rc = 1;
    return rc;
}




void initializeSquares(square* squares_table,int SQUARE_COUNT){
  
    // Initialising squares by user and randomly
    int selfinit_squares = 0;
    int table_size = 0;
    int k = 0;

    int s_x = 0;
    int s_y = 0;
    int s_speedx = 0;
    int s_speedy = 0;

    // Initialising squares by user and randomly
    while(true){
        printf("How many squares would you like to initalize yourself ?\n");
        scanf("%d",&selfinit_squares);

        if(selfinit_squares > SQUARE_COUNT)
            printf("You can't initialise more than the number of squares, please try again\n");
        else
            break;
    }

    while(k < selfinit_squares) {
        printf("Square number %d \n",k+1);
        printf("Please introduce values for the following variables \n");
        printf("x = ");
        scanf("%d",&s_x);
        printf("y = ");
        scanf("%d",&s_y);
        printf("speedx = ");
        scanf("%d",&s_speedx);
        printf("speedy = ");
        scanf("%d",&s_speedy);


        square new_square = {.x = s_x, .y = s_y, .speedx = s_speedx, .speedy = s_speedy, .color = k % 4};

        // Check if the coordinates are in the bounds of the grid
        if(s_x + SQUARE_WIDTH <= SIZE_X && s_y + SQUARE_WIDTH <= SIZE_Y) {
            for(int j = 0; j < table_size; j++){
                if(hasIntersection(new_square, squares_table[j])) { // Check intersection with other squares
                printf("Squares overlap, please enter new value\n");
                    continue;
                }
            }
        }else{
            printf("Square out of bounds, please enter new values\n");
            continue;
        }

        squares_table[k] = new_square;
        table_size++;
        k++;

    }



    k = selfinit_squares;

    // Randomly generate the (remaining) squares
    while(k < SQUARE_COUNT) {

        srand(time(NULL));

        square new_square = {
        	.x = rand()%(SIZE_X - SQUARE_WIDTH),
            .y = rand()%(SIZE_Y - SQUARE_WIDTH),
            .speedx = rand()% 3 -1,
            .speedy = rand()%3 -1,
            .color = k % 4
    	   };

        // If no square initialised by the user we push the first random square
        if(k == 0){
            squares_table[0] = new_square;
            k++;
            continue;
        }

        for(int j = 0; j < k; j++){
            // Check intersection with other squares
            if(hasIntersection(new_square, squares_table[j])){
                break;
            }
            else {
                squares_table[k] = new_square;
                k++;
            }
        }
    }

  return;
}


/********************************************************MAIN******************************************/

int main(int argc, char** argv){

	int workers_semid;
	int access_semid;
	int posUpdated_semid;
  int shmid;
	key_t key_sem_workers, key_sem_access, key_sem_posUpdated;
	key_t key_shm;
	pid_t pid;
	point *segptr;

	//Asking how much squares user wants
	printf("How many squares running ?\n");
	int SQUARE_COUNT = 0;
	scanf("%d", &SQUARE_COUNT);
    if(SQUARE_COUNT == 0){
        printf("Can't have 0 squares\n");
        return 0;
    }

	//Initialize SQUARE_COUNT number of squares
	square squares_table[SQUARE_COUNT];
	initializeSquares(squares_table,SQUARE_COUNT);


    key_sem_access = ftok(".", 'A');
    key_sem_workers = ftok(".", 'W');
    key_sem_posUpdated = ftok(".",'U');
    key_shm = ftok(".",'S');
    //We need to put the square table in shared memory, as well
   	// as finish 
    int shmsize = SQUARE_COUNT*sizeof(square) + 1;

	/* Open the shared memory segment - create if necessary */
    if((shmid = shmget(key_shm,shmsize, IPC_CREAT|IPC_EXCL|0666)) == -1) {
            printf("Shared memory segment exists - opening as client\n");

            /* Segment probably already exists - try as a client */
            if((shmid = shmget(key_shm,shmsize, 0)) == -1) 
            {
                    perror("shmget");
                    exit(1);
            }
    }
    else{
            printf("Creating new shared memory segment\n");
    }

    /* Attach (map) the shared memory segment into the current process */
    if((segptr = (point*)shmat(shmid, 0, 0)) == (point*)-1){
            perror("shmat");
            exit(1);
    }


	//Creating a semaphore set with SQUARE_COUNT members
	createsem(&workers_semid, key_sem_workers, SQUARE_COUNT);
	setall(workers_semid,0);
	//Create a mutex to use when all workers have updated their positions
	createsem(&posUpdated_semid, key_sem_posUpdated, 1);
	setval(posUpdated_semid,0,0);

	//Create a mutex for the access to the square table
	createsem(&access_semid,key_sem_access ,1);
	setval(access_semid,0,0);


    int id = 1;

    //Put the squares position into shared memory
    for(id = 1; id <= SQUARE_COUNT; id++){
        point position = {.x = squares_table[id-1].x, .y = squares_table[id-1].y};
        writeshm(segptr,id,position);
    }

    point finish = {.x = 0,.y = 0};
    writeshm(segptr,0,finish); //finish = 0;

    //Creating SQUARE_COUNT workers
	for(int cntr = 0,id = 1; cntr < SQUARE_COUNT; cntr++)
	{
		pid = fork();
 
		if(pid < 0)
		{
			perror("Process creation failed");
			exit(1);
		}
		if(pid == 0)
		{
			//This is a son
			int speedx = squares_table[id-1].speedx;
			int speedy = squares_table[id-1].speedy;
			worker(id,SQUARE_COUNT,segptr,workers_semid,access_semid,posUpdated_semid,speedx,speedy);
      cntr = SQUARE_COUNT;
		}
		else
		{
			//This is the father
			id++;
		}
	}   


  int table_of_pixels[SIZE_X][SIZE_Y];  //Will store the states of the pixels

    for(id = 1; id <= SQUARE_COUNT; id++){
        for(int j = 0; j < SQUARE_WIDTH; j++){
            for(int k = 0; k < SQUARE_WIDTH; k++){
                point position = readshm(segptr,id);
                table_of_pixels[position.x+j][position.y+k] = (id-1) % 4;
            }
        }
    }

	//Initializes SDL and the colours
    init_output();
    printf("Initialized\n");



	//We enter the master_process code
	master_process(segptr,SQUARE_COUNT,workers_semid,access_semid,posUpdated_semid);

  control_process(SQUARE_COUNT+2, SQUARE_COUNT+1, SQUARE_COUNT);

	
	return 0;
}
