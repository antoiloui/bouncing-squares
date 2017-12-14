//Including system libraries
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include<stdbool.h>
/*#include <termios.h>
#include <unistd.h>
#include <fcntl.h>*/

//Including user libraries
#include "constants.h"
//#include "output.h"

//Custom structure
struct square_t
{
  int x;
  int y;
  int color;
  int speedx;
  int speedy;
};
typedef struct square_t square;


//Global variables
int table_of_pixels[SIZE_X][SIZE_Y];	//Will store the states of the pixels
int temporary_table[SIZE_X][SIZE_Y];  //Will be used to store state n+1


//Method signatures
//void process(int iterations);					//Process the automaton for a given number of iterations
//int kbhit(void);			      			//Returns 1 if the user pressed a key and 0 otherwise
int hasIntersection(square a, square b);                        //Returns 1 if the two squares intersect and 0 otherwise

//Do two squares have an intersection?
int hasIntersection(square a, square b)
{
  int rc = 0;
  
  if(a.y < b.y+SQUARE_WIDTH && a.y+SQUARE_WIDTH > b.y &&
     a.x < b.x+SQUARE_WIDTH && a.x+SQUARE_WIDTH > b.x)
    rc = 1;
  return rc;
}



//Main function
int main(int argc, char** argv) {
  
  int iterations=-1;
	
	//The eventual parameter is the number of iterations
	if(argc > 1)
		iterations=atoi(argv[1]);

	
	//Filling the table with zeroes
	for(int i = 0; i < SIZE_X; ++i)
	{
		for(int j = 0; j < SIZE_Y; ++j)
		{
			table_of_pixels[i][j] = 0;
			temporary_table[i][j] = 0;
		}
	}

	//Asking how much squares user wants
	printf("How many squares running ?\n");
	int SQUARE_COUNT = 0;
	scanf("%d", &SQUARE_COUNT);

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
	int s_color = 0;

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
		printf("color = ");
		scanf("%d",&s_color);

		int table_size = 0;

		square new_square = {.x = s_x, .y = s_y, .speedx = s_speedx, .speedy = s_speedy, .color = s_color};

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
					 	   	 .color = rand()%4
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



	//Drawing the squares on the map
	for(int i = 0; i < SQUARE_COUNT; i++)
	{
	  for(int j = 0; j < SQUARE_WIDTH; j++)
	  {
	    for(int k = 0; k < SQUARE_WIDTH; k++)
	    {
	      table_of_pixels[squares_table[i].x+j][squares_table[i].y+k] = squares_table[i].color;
	    }
	  }
	}
	
//Initializes SDL and the colours
  init_output();
  printf("Initialized\n");
  
//Process the automaton for "iterations" iterations
  process(iterations);

//Get the user to press a key at the end of the iterations
  printf("Press a key to finish\n");
  getchar();

  return 1;
}



//Process the automaton for a number of iterations given as parameter
void master_process(int iterations) {
	
  int cnt = 0, i, j, k, temp;
  
  //While we haven't performed the required number of iterations (or if iterations == -1)
  while (iterations == -1 || cnt < iterations) {
    
    //Display
    printf("\nEnter next cycle\n");
    printf("Compute next table\n");
    
   
    
    //If the user hits a key, we exit the loop
    if(kbhit())
    {
    	iterations = 1;
    	cnt = iterations;
    }

	//Apply the change on SDL display
    update_output(table_of_pixels);
    
    //Wait a bit
    usleep(15000);
    
    //Increase the counter
    cnt++;
  }
}


void worker_process(id, speedx, speedy){

	int x_next,y_next;

	//Get next position
	x_next = squares_table[id].x + speedx;
	y_next = squares_table[id].y + speedy;

	//Check out of bounds




}



//Returns 1 if the user pressed a key, and 0 otherwise
int kbhit(void)
{
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
  if(ch != EOF)
  {
  	if(ch == '\n'){
  		//Put back the character on the input stream
	    ungetc(ch, stdin);
	    return 1;
  	}
  }
 
  return 0;
}





  
 








































  return 1;
}