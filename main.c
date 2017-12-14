// Square bounce by S. Hiard
// Partially reusing heatbugs level generator by A. Linden

//Including system libraries
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

//Including user libraries
#include "constants.h"
#include "output.h"

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
square squares_table[SQUARE_COUNT];                //To store position and velocities of squares

//Method signatures
void process(int iterations);					//Process the automaton for a given number of iterations
int kbhit(void);			      			//Returns 1 if the user pressed a key and 0 otherwise
int hasIntersection(square a, square b);                        //Returns 1 if the two squares intersect and 0 otherwise

//Does two squares have an intersection?
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
  
  int i, j, k, iterations=-1;
	
	//The eventual parameter is the number of iterations
	if(argc > 1)
		iterations=atoi(argv[1]);
	
	//Filling the table with zeroes
	for(i = 0; i < SIZE_X; ++i)
	{
		for(j = 0; j < SIZE_Y; ++j)
		{
			table_of_pixels[i][j] = 0;
			temporary_table[i][j] = 0;
		}
	}

	//Filling the table of squares
	squares_table[0].x = 0;
	squares_table[0].y = 16;
	squares_table[0].speedx = 1;
	squares_table[0].speedy = 1;
	squares_table[0].color = 1;

	squares_table[1].x = 32;
	squares_table[1].y = 64;
	squares_table[1].speedx = 0;
	squares_table[1].speedy = 1;
	squares_table[1].color = 2;

	squares_table[2].x = 12;
	squares_table[2].y = 56;
	squares_table[2].speedx = 1;
	squares_table[2].speedy = 1;
	squares_table[2].color = 3;

	squares_table[3].x = 32;
	squares_table[3].y = 156;
	squares_table[3].speedx = 1;
	squares_table[3].speedy = -1;
	squares_table[3].color = 4;

	for(i = 0; i < SQUARE_COUNT; i++)
	{
	  for(j = 0; j < SQUARE_WIDTH; j++)
	  {
	    for(k = 0; k < SQUARE_WIDTH; k++)
	    {
	      table_of_pixels[squares_table[i].x+j][squares_table[i].y+k] = squares_table[i].color;
	    }
	  }
	}
	
	//Initializes SDL and the colours
  init_output();
  printf("initialized\n");
  
	//Process the automaton for "iterations" iterations
  process(iterations);

	//Get the user to press a key at the end of the iterations
	printf("Press a key to finish\n");
  getchar();

  return 1;
}

//Process the automaton for a number of iterations given as parameter
void process(int iterations) {
	
  int cnt = 0, i, j, k, temp;
  
  //While we haven't performed the required number of iterations (or if iterations == -1)
  while (iterations == -1 || cnt < iterations) {
    
    //Display
    printf("\nenter next cycle\n");
    printf("compute next table\n");
    
    //We try to move each square
    for(i = 0; i < SQUARE_COUNT; i++)
    {
      squares_table[i].x += squares_table[i].speedx;

      //Correcting out of bounds
      if(squares_table[i].x > SIZE_X-SQUARE_WIDTH)
      {
	squares_table[i].x = SIZE_X-SQUARE_WIDTH;
	squares_table[i].speedx = -1;
      }
      if(squares_table[i].x < 0)
      {
	squares_table[i].x = 0;
	squares_table[i].speedx = 1;
      }

      squares_table[i].y += squares_table[i].speedy;

      //Correcting out of bounds
      if(squares_table[i].y > SIZE_Y-SQUARE_WIDTH)
      {
	squares_table[i].y = SIZE_Y-SQUARE_WIDTH;
	squares_table[i].speedy = -1;
      }
      if(squares_table[i].y < 0)
      {
	squares_table[i].y = 0;
	squares_table[i].speedy = 1;
      }
    }

      //See if there's a collision between squares
    for(i = 0; i < SQUARE_COUNT; i++)
    {
      for(j=i+1; j < SQUARE_COUNT; j++)
      {
	if(hasIntersection(squares_table[i],squares_table[j]))
	{
	    squares_table[i].x -= squares_table[i].speedx;
	    squares_table[i].y -= squares_table[i].speedy;
            if(hasIntersection(squares_table[i],squares_table[j]))
	    {
	      squares_table[j].x -= squares_table[j].speedx;
	      squares_table[j].y -= squares_table[j].speedy;
	    }
	  temp = squares_table[i].speedx;
	  squares_table[i].speedx = squares_table[j].speedx;
	  squares_table[j].speedx = temp;

	  temp = squares_table[i].speedy;
	  squares_table[i].speedy = squares_table[j].speedy;
	  squares_table[j].speedy = temp;

	}
      }
    }

    //Updating table of pixels
    for(i = 0; i < SIZE_X; i++)
    {
      for(j = 0; j < SIZE_Y; j++)
      {
	table_of_pixels[i][j] = 0;
      }
    }
    for(i = 0; i < SQUARE_COUNT; i++)
	{
	  for(j = 0; j < SQUARE_WIDTH; j++)
	  {
	    for(k = 0; k < SQUARE_WIDTH; k++)
	    {
	      table_of_pixels[squares_table[i].x+j][squares_table[i].y+k] = squares_table[i].color;
	    }
	  }
	}
    
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
  	//Put back the character on the input stream
    ungetc(ch, stdin);
    return 1;
  }
 
  return 0;
}

