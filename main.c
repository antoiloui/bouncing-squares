//Including system libraries
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

//Including user libraries
#include "process_communication.h"
#include "constants.h"
#include "output.h"


/************************************PROTOTYPES****************************************************/
int hasIntersection(point a, point b); //Returns 1 if the two squares intersect and 0 otherwise
void initializeSquares(square* squares_table,int SQUARE_COUNT); //Initialize the squares
int kbhit(void);//Returns 1 if the user pressed a key, and 0 otherwise



/*****************************************PROCESSES**********************************************/



control_process(point* segptr, int workers_semid, int access_semid, int posUpdated_semid, int collision_semid, int msgq_id, int shmid){
    char c;
    point finish;

    if(kbhit()){
        printf("Enter pressed, program exited.\n");
        finish.x = 1;
        writeshm(segptr,0,finish);

        // Close messages queues
        remove_queue(msgq_id);

        // Close shared memory
        remove_shm(shmid);

        //Close semaphores
        remove_sem(workers_semid);
        remove_sem(access_semid);
        remove_sem(posUpdated_semid);
        remove_sem(collision_semid);
    }
}
  
  


master_process(point* segptr,int SQUARE_COUNT, int workers_semid, int access_semid, int posUpdated_semid, int collision_semid) {

  int table_of_pixels[SIZE_X][SIZE_Y];  //Will store the states of the pixels
  int id,j,k;
  point allUpdated;
  
    //As long as the user doesn't quit
    while(readshm(segptr,0).x != 1) {
    
        //Display
        printf("\nEnter next cycle\n");
        printf("Compute next table\n");


        for(int id = 1; id <= SQUARE_COUNT; id++){
            unlocksem(workers_semid,id-1);
        }

        unlocksem(access_semid,0); //Give access to the square table
        printf("Access given to one worker\n");


        //Wait before all workers have updated their position
        for(int cntr = 0; cntr < SQUARE_COUNT ; cntr++) {
            locksem(posUpdated_semid,0);
            printf("Update\n");

        }
        printf("All workers updated their position\n");

        //Set allUpdated to true
        allUpdated.x = 1;
        writeshm(segptr,SQUARE_COUNT+1,allUpdated); 

        for(id = 1; id <= SQUARE_COUNT; id++){ //Unlock all semaphores waiting for collision
            printf("Unlocking collision\n");
            unlocksem(collision_semid,id-1);

        }
        printf("All collisions unlocked\n");


        //Updating the table_of_pixels
        for(j = 0; j < SIZE_X; j++){
            for(k = 0; k < SIZE_Y; k++){
                table_of_pixels[j][k] = 0;
            }
        }

        for(id = 1; id <= SQUARE_COUNT; id++){
            point position = readshm(segptr,id);
            printf("Position of square %d : (%d,%d)\n",id,position.x,position.y);
            for(j = 0; j < SQUARE_WIDTH; j++){
                for(k = 0; k < SQUARE_WIDTH; k++){
                    table_of_pixels[position.x+j][position.y+k] = id%4 +1;
                }
            }
        }

        //Apply the change on SDL display
        update_output(table_of_pixels);
        //Wait a bit
        usleep(1000);    
    }
}



worker(int id, int SQUARE_COUNT, point* segptr, int workers_semid, int access_semid, int posUpdated_semid,int collision_semid,int msgq_id,struct mymsgbuf *qbuf, int speedx, int speedy){

    point next_pos;
    point current_pos;

    while(readshm(segptr,0).x != 1) {

        printf("Worker %d is working\n", id);
        locksem(access_semid,0); //wait(accessPositio   nTable)
        //Get current position
        printf("Worker %d computing position\n", id);

        current_pos = readshm(segptr,id);
        //Compute next position
        next_pos.x = current_pos.x + speedx;
        next_pos.y = current_pos.y + speedy;

        //Collision with right wall
        if(next_pos.x >  SIZE_X-SQUARE_WIDTH){
            next_pos.x = SIZE_X-SQUARE_WIDTH;
            speedx = -1;
        }

        //Collision with left wall
        if(next_pos.x < 0){
            next_pos.x = 0;
            speedx = 1;
        }

        //Collision with bottom wall
        if(next_pos.y > SIZE_Y-SQUARE_WIDTH){
            next_pos.y = SIZE_Y-SQUARE_WIDTH;
            speedy = -1;
        }

        //Collision with top wall
        if(next_pos.y < 0){
            next_pos.y = 0;
            speedy = 1;
        }

        for(size_t other_id = 1; other_id <= SQUARE_COUNT; other_id++){
            if(other_id != id){
                point other_pos = readshm(segptr,other_id);
                if(hasIntersection(next_pos,other_pos)){
                    if(getval(workers_semid,other_id) == 0){ //if the other has updated it's position
                        printf("Worker %d collided with worker %d \n", id, other_id);

                        unlocksem(collision_semid,other_id-1);//signal(collision_id)
                        struct speed_s speed = {.speed_x = speedx, .speed_y = speedy};
                        send_message(msgq_id,qbuf, other_id, id,speed);
                        read_message(msgq_id,qbuf, id, other_id);
                        speedx = qbuf->speed.speed_x;
                        speedy = qbuf->speed.speed_y;
                        next_pos.x = current_pos.x + speedx;
                        next_pos.y = current_pos.y + speedy;

                    }
                }          
            }
        }




        writeshm(segptr,id,next_pos); //Update position

        unlocksem(access_semid,0);//signal(accessPositionTable)
        printf("Worker %d has released access\n", id);

        unlocksem(posUpdated_semid,0); //has updated it's position
        printf("Worker %d position updated\n", id);

        locksem(collision_semid,id-1); // Wait for collision
        printf("Collision semaphore unlocked (a)\n");

        while(readshm(segptr,SQUARE_COUNT+1).x == 0){
            struct speed_s speed = {.speed_x = speedx, .speed_y = speedy};
            int other_id = qbuf->receiver;
            read_message(msgq_id,qbuf,other_id,id); //Read speed
            speedx = qbuf->speed.speed_x; //Update speed
            speedy = qbuf->speed.speed_y;
            send_message(msgq_id,qbuf, id, other_id,speed); //Send speed

            locksem(collision_semid,id-1); // Wait for collision
            printf("Collision semaphore unlocked (b)\n");

        }

        printf("Waiting to reactivate\n");

        locksem(workers_semid,id-1); // Wait for the master process

    }
}




/******************************************FUNCTIONS***************************************************/


int hasIntersection(point a, point b){
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
        printf("How many squares would you like to initalize yourself ? Please introduce an integer.\n");
        scanf("%d",&selfinit_squares);

        if(selfinit_squares > SQUARE_COUNT)
            printf("You can't initialise more squares than the predefined number of squares, please try again.\n");
        else
            break;
    }

    while(k < selfinit_squares) {
        printf("Square number %d \n",k+1);
        printf("Please introduce integer values for the following variables :\n");
        printf("x = ");
        scanf("%d",&s_x);
        printf("y = ");
        scanf("%d",&s_y);
        printf("speedx = ");
        scanf("%d",&s_speedx);
        printf("speedy = ");
        scanf("%d",&s_speedy);


        square new_square = {.x = s_x, .y = s_y, .speedx = s_speedx, .speedy = s_speedy};
        point new_square_position = {.x = s_x, .y = s_y};

        // Check if the coordinates are in the bounds of the grid
        if(s_x + SQUARE_WIDTH <= SIZE_X && s_y + SQUARE_WIDTH <= SIZE_Y) {
            for(int j = 0; j < table_size; j++){
                point square_position = {.x = squares_table[j].x, .y = squares_table[j].y};
                if(hasIntersection(new_square_position,square_position)) { // Check intersection with other squares
                    printf("Squares overlap, please enter new integer value.\n");
                    continue;
                }
            }
        }else{
            printf("Square out of bounds, please enter new integer values.\n");
            continue;
        }

        squares_table[k] = new_square;
        table_size++;
        k++;

    }

    k = selfinit_squares;

    // Randomly generate the (remaining) squares
    while(k < SQUARE_COUNT) {

        //rand()%(max-min)+min
        square new_square = {
        	.x = rand()%(SIZE_X - SQUARE_WIDTH),
            .y = rand()%(SIZE_Y - SQUARE_WIDTH),
            .speedx = rand()% 3 -1,
            .speedy = rand()%3 -1,
            .color = k%4 +1
    	   };

        // If no square initialised by the user we push the first random square
        if(k == 0){
            squares_table[0] = new_square;
            k++;
            continue;
        }

        point new_square_position = {.x = new_square.x, .y = new_square.y};

        for(int j = 0; j < k; j++){
            point square_position = {.x = squares_table[j].x, .y = squares_table[j].y};

            // Check intersection with other squares
            if(hasIntersection(new_square_position, square_position)){
                printf("New square has intersection, new square is being created.\n");
                break;
            }
            else{
                printf("New square does not have intersection, ok !\n");
                squares_table[k] = new_square;
            }
        }
        k++;
    }
}


int kbhit(void){
    struct termios oldt, newt;
    int ch;
    int oldf;

    //Changing the flags to make getchar() a non blocking operation
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    //Try to read of character (non-blocking)
    ch = getchar();

    //Resetting the flags to their old values
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    //If we did manage to read something
    if(ch != EOF){
        if(ch == 'a'){
            //Put back the character on the input stream
            ungetc(ch, stdin);
            return 1;
        }
    }

    return 0;
}


/********************************************************MAIN******************************************/

int main(int argc, char** argv){

	int workers_semid, access_semid, posUpdated_semid, collision_semid;
    int msgq_id;
    int shmid;
	key_t key_sem_workers, key_sem_access, key_sem_posUpdated, key_sem_collision;
	key_t key_shm;
    key_t key_q;
	pid_t pid;
    struct mymsgbuf qbuf;
	point *segptr;


    srand(time(NULL));

	//Asking how much squares user wants
    int SQUARE_COUNT = 0;

    while(true){
        printf("How many squares running ? Please introduce a non-null integer.\n");
        scanf("%d", &SQUARE_COUNT);

        if(SQUARE_COUNT == 0)
            printf("Can't have 0 square, please introduce a non-null integer.\n");
        else
            break;
    }
    
	//Initialize SQUARE_COUNT number of squares
	square squares_table[SQUARE_COUNT];
	initializeSquares(squares_table,SQUARE_COUNT);


    key_sem_access = ftok(".", 'A');
    key_sem_workers = ftok(".", 'W');
    key_sem_posUpdated = ftok(".",'U');
    key_sem_collision = ftok(".",'C');
    key_shm = ftok(".",'S');
    key_q = ftok(".", 'Q');

    //We need to put the square table in shared memory, as well
   	// as finish [finish, SQ1, SQ2, SQ3, ...] and allUpdated
    int shmsize = SQUARE_COUNT*sizeof(square) + 2;

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

    //Create a sempahore to signal collision with another square
    createsem(&collision_semid,key_sem_collision ,SQUARE_COUNT);
    setall(collision_semid,0);

    //Create a message queue for workers to exchange their speeds
    createqueue(&msgq_id, key_q, 1);




    point finish = {.x = 0,.y = 0};
    writeshm(segptr,0,finish); //finish = 0;

    int id = 1;

    //Put the squares position into shared memory
    for(id = 1; id <= SQUARE_COUNT; id++){
        point position = {.x = squares_table[id-1].x, .y = squares_table[id-1].y};
        writeshm(segptr,id,position);
    }


    point allUpdated = {.x = 0,.y = 0}; // allUpdated is false
    writeshm(segptr,SQUARE_COUNT + 1,allUpdated); //finish = 0;

    //Creating SQUARE_COUNT workers
	for(int cntr = 0,id = 1; cntr < SQUARE_COUNT+1 ; cntr++){
		
        pid = fork();
 
		if(pid < 0){
			perror("Process creation failed");
			exit(1);
		}
		if(pid == 0){
            //The last son is the control process
            if(cntr < SQUARE_COUNT){
                //This is a son
                int speedx = squares_table[id-1].speedx;
                int speedy = squares_table[id-1].speedy;
                worker(id,SQUARE_COUNT,segptr,workers_semid,access_semid,posUpdated_semid,collision_semid,msgq_id,&qbuf,speedx,speedy);

            }
            else
                control_process(segptr, workers_semid, access_semid, posUpdated_semid, collision_semid, msgq_id, shmid);



            cntr = SQUARE_COUNT +1;

		}
		else{
			//This is the father
			id++;
		}
	}   

    int table_of_pixels[SIZE_X][SIZE_Y];  //Will store the states of the pixels

    for(id = 1; id <= SQUARE_COUNT; id++){
        for(int j = 0; j < SQUARE_WIDTH; j++){
            for(int k = 0; k < SQUARE_WIDTH; k++){
                point position = readshm(segptr,id);
                table_of_pixels[position.x+j][position.y+k] = id%4 +1;
            }
        }
    }

	//Initializes SDL and the colours
    init_output();
    printf("Initialized\n");

	//We enter the master_process code
	master_process(segptr,SQUARE_COUNT,workers_semid,access_semid,posUpdated_semid,collision_semid);
	
	return 0;
}
