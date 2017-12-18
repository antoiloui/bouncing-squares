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

//Including user libraries
#include "process_communication.h"
#include "constants.h"
#include "output.h"


/************************************************************************PROTOTYPES*********************************************************************************************/


/************************************************************************
* Returns 1 if two squares intersect, 0 otherwise
*
* ARGUMENTS:
* -Position of two squares given by the point structure
**************************************************************************/
int hasIntersection(point a, point b);


/************************************************************************
* Returns 1 if a square intersects with any squares 
* in the squares_table array
*
* ARGUMENTS:
* -Position of a squares 
* -pointer to squares_table
* -size of the squares_table
**************************************************************************/
bool square_intersected(square* squares_table, square new_square, int k); 


/************************************************************************
* Creates SQUARE_COUNT squares and puts them in the squares_table array
* 
* ARGUMENTS:
* -pointer to squares_table
* -number of squares to initialize
*************************************************************************/
void initializeSquares(square* squares_table,int SQUARE_COUNT); 


/************************************************************************
/* Returns 1 if the user pressed a key, and 0 otherwise
*************************************************************************/
int kbhit(void);


/************************************************************************
* Process managing the user input during the execution of the program 
* Does nothing until the user presses <Enter>
*
* ARGUMENTS:
* -pointer to the shared memory segment
* -number of squares initialised
* -IDs of all the semaphores sets (5)
* -ID of the message queue
* -ID of the shared memory segment
**************************************************************************/
void control_process(point* segptr, int SQUARE_COUNT, int workers_semid, int access_semid, int posUpdated_semid, int collision_semid, int control_semid,int msgq_id, int shmid);


/************************************************************************
* Process managing the different workers and update the table of pixels with new positions
*
* ARGUMENTS:
* -pointer to the shared memory segment
* -number of squares initialised
* -IDs of all the semaphores sets (5)
**************************************************************************/
void master_process(point* segptr,int SQUARE_COUNT, int workers_semid, int access_semid, int posUpdated_semid, int collision_semid, int control_semid);


/************************************************************************
* Process calculating the new position of a square
* and managing eventual  collisions 
*
* ARGUMENTS:
* -ID of the square
* -number of squares initialised
* -pointer to the shared memory segment
* -IDs of 4 semaphore sets (except control_semid)
* -ID of the message queue
* -Speed of the square among the x- and y-axis
**************************************************************************/
void worker(int id, int SQUARE_COUNT, point* segptr, int workers_semid, int access_semid, int posUpdated_semid,int collision_semid,int msgq_id, int speedx, int speedy);







/*************************************************************************PROCESSES********************************************************************************************/


void control_process(point* segptr, int SQUARE_COUNT, int workers_semid, int access_semid, int posUpdated_semid, int collision_semid, int control_semid,int msgq_id, int shmid){

    point finish = {.x = 1};
    bool stop = false;

    printf("Control process initialised.\n");

    while(!stop){
    
        if(kbhit()){
            stop = true;

            writeshm(segptr,0,finish);

            //Unlock the master process (it will make one lats iteration and unlock all the workers that will then quit their loop)
            for(int id = 1; id <= SQUARE_COUNT; id++){
                unlocksem(posUpdated_semid,0);
            }

            // Wait that the master process finish its iteration
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

    printf("Exiting the control process.\n");
}
  




void master_process(point* segptr,int SQUARE_COUNT, int workers_semid, int access_semid, int posUpdated_semid, int collision_semid, int control_semid){

    int table_of_pixels[SIZE_X][SIZE_Y];  //Will store the states of the pixels
    int id,j,k;
    point allUpdated;

    printf("Master process initialised.\n");

    unlocksem(access_semid,0); //Give access to the square table

    //As long as the user doesn't quit
    while(readshm(segptr,0).x != 1) {


        //Wait before all workers have updated their position
        for(int cntr = 0; cntr < SQUARE_COUNT ; cntr++) {
            locksem(posUpdated_semid,0);
        }

        //Set allUpdated to true
        allUpdated.x = 1;
        writeshm(segptr,2*SQUARE_COUNT+1,allUpdated);         

        //Unlock all semaphores waiting for collision
        for(id = 1; id <= SQUARE_COUNT; id++){ 
            unlocksem(collision_semid,id-1);
        }

        //Updating the table_of_pixels
        for(j = 0; j < SIZE_X; j++){
            for(k = 0; k < SIZE_Y; k++){
                table_of_pixels[j][k] = 0;
            }
        }

        for(id = 1; id <= SQUARE_COUNT; id++){
            point position = readshm(segptr,id);
            for(j = 0; j < SQUARE_WIDTH; j++){
                for(k = 0; k < SQUARE_WIDTH; k++){
                    table_of_pixels[position.x+j][position.y+k] = id%4 +1;
                }
            }
        }

        //Apply the change on SDL display
        update_output(table_of_pixels);

        point isUpdated = {.x = 0}; //Set isUpdated tag for every worker to false
        for(id = 1; id <= SQUARE_COUNT; id++){
            writeshm(segptr,SQUARE_COUNT+id,isUpdated);
        }

        //Set allUpdated to false
        allUpdated.x = 0;
        writeshm(segptr,2*SQUARE_COUNT+1,allUpdated); 

        //Wait a bit
        usleep(15000); 

        //Unlock all the workers when they are all updated (bottom of the worker process)
        for(id = 1; id <= SQUARE_COUNT; id++){
            unlocksem(workers_semid,id-1);
        }
        
    }

    //Wait that the last worker quit its loop and then unlock the control_semid (control process continue)
    usleep(50000);
    unlocksem(control_semid,0);

    printf("Exit the master process.\n");
}





void worker(int id, int SQUARE_COUNT, point* segptr, int workers_semid, int access_semid, int posUpdated_semid,int collision_semid,int msgq_id, int speedx, int speedy){

    point next_pos;
    point current_pos;

    printf("Worker %d initialised.\n", id);

    while(readshm(segptr,0).x != 1) {


        locksem(access_semid,0); //wait(accessPositionTable)
        
        //Get current position
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

        for(int other_id = 1; other_id <= SQUARE_COUNT; other_id++){
            if(other_id != id){

                point other_pos = readshm(segptr,other_id);

                if(hasIntersection(next_pos,other_pos)){

                    if(readshm(segptr,SQUARE_COUNT+other_id).x == 1){ //if the other has updated it's position
                        printf("Worker %d collided with worker %d \n", id, other_id);
                        unlocksem(collision_semid,other_id-1);//signal(collision_id)
                       
                        struct speed_s send_speed = {.speed_x = speedx, .speed_y = speedy};
                        struct mymsgbuf sendbuf ={.type = other_id, .sender = id,.speed = send_speed};

                        send_message(msgq_id,&sendbuf);
                        struct mymsgbuf receivebuf;

                        read_message(msgq_id,&receivebuf, id);

                        //If the received speed is already the speed of the square
                        if(receivebuf.speed.speed_x == speedx && receivebuf.speed.speed_y == speedy){
                            // Change it to the opposite speed
                            speedx *= - 1;
                            speedy *= - 1;
                        }else{
                            // Change with the speed of the other square
                            speedx = receivebuf.speed.speed_x;
                            speedy = receivebuf.speed.speed_y;
                        }

                        next_pos.x = current_pos.x + speedx;
                        next_pos.y = current_pos.y + speedy;

                        other_id = 0;
                        continue;
                    }
                }          
            }
        }

        writeshm(segptr,id,next_pos); //Update position
        point isUpdated = {.x = 1}; //Set isUpdated to true
        writeshm(segptr,SQUARE_COUNT+id,isUpdated);

        unlocksem(access_semid,0);//signal(accessPositionTable)

        unlocksem(posUpdated_semid,0); //has updated it's position/ is also counter

        locksem(collision_semid,id-1); // Wait for collision


        while(readshm(segptr,2*SQUARE_COUNT+1).x == 0){

            struct mymsgbuf receivebuf; //Container to receive speed

            read_message(msgq_id,&receivebuf,id); //Read speed

            long other_id = (long)receivebuf.sender; //Get the id of the sender

            struct speed_s send_speed = {.speed_x = speedx, .speed_y = speedy};
            struct mymsgbuf sendbuf ={.type = other_id, .sender = id,.speed = send_speed};

            speedx = receivebuf.speed.speed_x; //Update speed
            speedy = receivebuf.speed.speed_y;
            
            send_message(msgq_id,&sendbuf); //Send speed back to sender

            locksem(collision_semid,id-1); // Wait for collision
        }

        locksem(workers_semid,id-1); // Wait for the master process

    }

    printf("Exit the worker number %d.\n", id);
}




/***************************************************************************METHODS************************************************************************************/


int hasIntersection(point a, point b){
    int rc = 0;
  
    if(a.y < b.y+SQUARE_WIDTH && a.y+SQUARE_WIDTH > b.y &&
        a.x < b.x+SQUARE_WIDTH && a.x+SQUARE_WIDTH > b.x)
        rc = 1;

    return rc;
}



bool square_intersected(square* squares_table, square new_square, int k){

    point new_square_position = {.x = new_square.x, .y = new_square.y};

    for(int j = 0; j < k; j++){

        point square_position = {.x = squares_table[j].x, .y = squares_table[j].y};

        if(hasIntersection(new_square_position, square_position))
            return true;
    }
    return false;
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
        printf("How many squares would you like to initalize yourself ? Please introduce a positive integer.\n");
        if(scanf("%d",&selfinit_squares) != 1){
            printf("You didn't enter a number, please try again.\n");
            while(getchar() != '\n');
            continue;
        }

        if(selfinit_squares > SQUARE_COUNT)
            printf("You can't initialise more squares than the predefined number of squares, please try again.\n");
        else if(selfinit_squares < 0)
            printf("You entered a negative number, please try again.\n");
        else
            break;
    }

    while(k < selfinit_squares) {
        printf("Square number %d \n",k+1);
        printf("Please introduce positive integer values for the position :\n");
        printf("x = ");
        if(scanf("%d",&s_x) != 1){
            printf("You didn't enter a number, please try again.\n");
            while(getchar() != '\n');
            continue;
        }
        printf("y = ");
        if(scanf("%d",&s_y) != 1){
            printf("You didn't enter a number, please try again.\n");
            while(getchar() != '\n');
            continue;
        }

        if(s_x < 0 || s_y < 0){
            printf("You entered negative values for the position, try again.\n");
            while(getchar() != '\n');
            continue;
        }

        printf("Please introduce -1, 0 or 1 for the speed :\n");
        printf("speedx = ");
        if(scanf("%d",&s_speedx) != 1){
            printf("You didn't enter a number, please try again.\n");
            while(getchar() != '\n');
            continue;
        }
        printf("speedy = ");
        if(scanf("%d",&s_speedy) != 1){
            printf("You didn't enter a number, please try again.\n");
            while(getchar() != '\n');
            continue;
        }

        if((s_speedx != -1 && s_speedx != 0 && s_speedx != 1) || (s_speedy != -1 && s_speedy != 0 && s_speedy != 1)){
            printf("You entered other values than -1, 0 or 1 for the speed, please try again.\n");
            continue;
        }

        square new_square = {.x = s_x, .y = s_y, .speedx = s_speedx, .speedy = s_speedy};

        // Check if the coordinates are in the bounds of the grid
        if(s_x + SQUARE_WIDTH <= SIZE_X && s_y + SQUARE_WIDTH <= SIZE_Y) {

            if(square_intersected(squares_table, new_square, k)){
                //If there is, we loop again
                printf("Square number %d has an intersection with another square, another one is being created.\n",k);
                continue;
            }else{
                //If there is none, we append the new square to the table
                printf("Square number %d is being created.\n", k);
                squares_table[k] = new_square;
                k++;
            }

        }else{
            printf("Square out of bounds, please enter new integer values.\n");
            continue;
        }

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

        if(square_intersected(squares_table, new_square, k)){
            //If there is, we loop again
            continue;
        }else{
            //If there is none, we append the new square to the table
            squares_table[k] = new_square;
            k++;
        } 
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
        if(ch == '\n'){
            printf("Enter pressed, program exited.\n");
            //Put back the character on the input stream
            ungetc(ch, stdin);
            return 1;
        }
    }

    return 0;
}


/********************************************************************************MAIN**************************************************************************/

int main(int argc, char** argv){

    int workers_semid, access_semid, posUpdated_semid, collision_semid, control_semid;
    int msgq_id;
    int shmid;

    key_t key_sem_workers, key_sem_access, key_sem_posUpdated, key_sem_collision, key_sem_control;
    key_t key_shm;
    key_t key_q;
    pid_t pid;
    point *segptr;


    srand(time(NULL));

    //Asking how much squares user wants
    int SQUARE_COUNT = 0;

    while(true){
        printf("How many squares running ? Please introduce a positive integer.\n");
        if(scanf("%d", &SQUARE_COUNT) != 1){
            printf("You didn't enter a number, please try again.\n");
            while(getchar() != '\n');
            continue;
        }

        if(SQUARE_COUNT <= 0)
            printf("Can't have 0 squares or less, please introduce a positive integer.\n");
        else
            break;
    }
    
    //Initialize SQUARE_COUNT number of squares
    square squares_table[SQUARE_COUNT];
    initializeSquares(squares_table,SQUARE_COUNT);


    //Clear the input buffer so that kbhit() works
    int c = 0;
    while ((c = getchar()) != '\n' && c != EOF);

    key_sem_access = ftok(".", 'A');
    key_sem_workers = ftok(".", 'W');
    key_sem_posUpdated = ftok(".",'U');
    key_sem_collision = ftok(".",'C');
    key_sem_control = ftok(".", 'L');
    key_shm = ftok(".",'S');
    key_q = ftok(".", 'Q');

    //We need to put the square table in shared memory, as well
    // as finish [finish, SQ1, SQ2, SQ3, ...isUpdated1,isUpdated2,...,allUpdated]
    int shmsize = 2*SQUARE_COUNT*sizeof(square) + 2;

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

    //Create a mutex to wait that master finish an iteration when enter is pressed
    createsem(&control_semid, key_sem_control,1);
    setall(control_semid,0);

    //Create a message queue for workers to exchange their speeds
    createqueue(&msgq_id, key_q, 1);


    //initialisation of the finish variable
    point finish = {.x = 0, .y = 0};
    writeshm(segptr,0,finish); //finish = 0;

    int id = 1;

    //Put the squares position into shared memory
    for(id = 1; id <= SQUARE_COUNT; id++){
        point position = {.x = squares_table[id-1].x, .y = squares_table[id-1].y};
        point isUpdated =  {.x = 0, .y = 0};
        writeshm(segptr,id,position);
        writeshm(segptr,SQUARE_COUNT+id,isUpdated);
    }


    point allUpdated = {.x = 0,.y = 0}; // allUpdated is false
    writeshm(segptr,2*SQUARE_COUNT + 1,allUpdated); //finish = 0;


    int table_of_pixels[SIZE_X][SIZE_Y];  //Will store the states of the pixels

    for(id = 1; id <= SQUARE_COUNT; id++){
        for(int j = 0; j < SQUARE_WIDTH; j++){
            for(int k = 0; k < SQUARE_WIDTH; k++){
                point position = readshm(segptr,id);
                table_of_pixels[position.x+j][position.y+k] = id%4 +1;
            }
        }
    }


    //Creating SQUARE_COUNT workers
    for(int cntr = 0,id = 1; cntr < SQUARE_COUNT+1 ; cntr++){
        
        pid = fork();
 
        if(pid < 0){
            perror("Process creation failed");
            exit(1);
        }
        if(pid == 0){

            if(cntr < SQUARE_COUNT){
                //This is a son
                int speedx = squares_table[id-1].speedx;
                int speedy = squares_table[id-1].speedy;
                worker(id,SQUARE_COUNT,segptr,workers_semid,access_semid,posUpdated_semid,collision_semid,msgq_id,speedx,speedy);
            }else{
                //Initializes SDL and the colours
                init_output();
                master_process(segptr,SQUARE_COUNT,workers_semid,access_semid,posUpdated_semid,collision_semid,control_semid);

            } 
            cntr = SQUARE_COUNT+1;
        }

        else{
            //This is the father
            id++;
        }
    }    
    if(pid != 0)
        control_process(segptr, SQUARE_COUNT, workers_semid, access_semid, posUpdated_semid, collision_semid,control_semid, msgq_id, shmid);


   
    return 0;
}