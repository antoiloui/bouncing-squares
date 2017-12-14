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

/************************************PROTOTYPES****************************************************/
int hasIntersection(square a, square b);  //Returns 1 if the two squares intersect and 0 otherwise
square* initializeSquares(square* squares_table,int SQUARE_COUNT);


/*****************************************PROCESSES**********************************************/

/*
void control_process(){
  while(getChar()){}
  removeshm(int shmid);
  removeshm(int shmid)
}
*/

master_process(point* segptr,int SQUARE_COUNT, int workers_semid, int access_semid, int posUpdated_semid) {
  int table_of_pixels[SIZE_X][SIZE_Y];  //Will store the states of the pixels

  int finish = 0;
  int id,j,k;
  
  //As long as the user doesn't quit
  while((finish = readshm(segptr,0).x) != 1) {
    
      //Display
      printf("\nEnter next cycle\n");
      printf("Compute next table\n");

    for(int id = 1; id <= SQUARE_COUNT; id++){
      unlocksem(workers_semid,id);
    }

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
      usleep(5000);

    
  }
}


void worker(int id, int SQUARE_COUNT, point* segptr, int workers_semid, int access_semid, int posUpdated_semid, int speedx, int speedy){
    point next_pos;
    point current_pos;
    int finish = 0;

  while((finish = readshm(segptr,0).x) != 1) {

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




square* initializeSquares(square* squares_table,int SQUARE_COUNT){
  
  // Initialising squares by user and randomly
  int selfinit_squares = 0;
  while(true){
    printf("How many squares would you like to initalize yourself ?\n");
    scanf("%d",&selfinit_squares);

    if(selfinit_squares > SQUARE_COUNT)
      printf("You can't initialise more than the number of squares, please try again\n");
    else
      break;
  }

  square squares_table[SQUARE_COUNT];        //To store position and velocities of squares

  int s_x = 0;
  int s_y = 0;
  int s = 0;
  int s_speedx = 0;
  int s_speedy = 0;

  int k = 0;
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


    int table_size = 0;

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

  // Randomly generate the (remaining) squares
  while(selfinit_squares < SQUARE_COUNT) {

    srand(time(NULL));

    square new_square = {.x = rand()%(SIZE_X - SQUARE_WIDTH),
               .y = rand()%(SIZE_Y - SQUARE_WIDTH),
               .speedx = rand()% 3 -1,
                 .speedy = rand()%3 -1,
                 .color = selfinit_squares % 4
                };

    for(int j = 0; j < selfinit_squares; j++){
       // Check intersection with other squares
      if(hasIntersection(new_square, squares_table[j])){
        break;
      }
      else {
        squares_table[selfinit_squares] = new_square;
        selfinit_squares++;
      }
    }
  }

  return squares_table;
}


/********************************************************MAIN******************************************/

int main(int argc, char** argv){

	int workers_semid;
	int access_semid;
	int posUpdated_semid;
	point *segptr;

	//Asking how much squares user wants
	printf("How many squares running ?\n");
	int SQUARE_COUNT = 0;
	scanf("%d", &SQUARE_COUNT);

	//Initialize SQUARE_COUNT number of squares
	square* squares_table;
	initializeSquares(squares_table,SQUARE_COUNT);

	key_t key_sem_workers, key_sem_access, key_sem_posUpdated;
	key_t key_shm;
	pid_t pid;
  int  shmid;

    key_sem_access = ftok(".", 'A');
    key_sem_workers = ftok(".", 'W');
    key_sem_posUpdated = ftok(".",'U');
    key_shm = ftok(".",'S');
    //We need to put the square table in shared memory, as well
   	// as finish 
    int shmsize = SQUARE_COUNT*sizeof(square) + 1;

	/* Open the shared memory segment - create if necessary */
    if((shmid = shmget(key,shmsize, IPC_CREAT|IPC_EXCL|0666)) == -1) {
            printf("Shared memory segment exists - opening as client\n");

            /* Segment probably already exists - try as a client */
            if((shmid = shmget(key,shmsize, 0)) == -1) 
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
	setAll(workers_semid,0);
	//Create a mutex to use when all workers have updated their positions
	createsem(&access_semid, key_sem_posUpdated, 1);
	setval(posUpdated_semid,0,0);

	//Create a mutex for the access to the square table
	createsem(&access_semid,key_sem_access ,1);
	setval(access_semid,0,0);


    //Creating SQUARE_COUNT workers
	int id = 1;
	for(int cntr = 0; cntr < SQUARE_COUNT; cntr++)
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


	//Initializes SDL and the colours
  	init_output();
  	printf("Initialized\n");


  	//Put the squares position into shared memory
  	for(id = 1; id <= SQUARE_COUNT; id++){
  		point position = {.x = squares_table[id-1].x, .y = squares_table[id-1].y};
		writeshm(segptr,id,position);
	}

	point finish = {.x = 0,.y = 0};
	writeshm(segptr,0,finish); //finish = 0;

	//We enter the master_process code
	master_process(segptr,workers_semid,access_semid,posUpdated_semid);

	
	return 1;
}


/*
void control_process(){
	while(getChar()){}
	removeshm(int shmid);
	removeshm(int shmid)
}
*/

