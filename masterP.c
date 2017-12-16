#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include"constants.h"
#include"output.h"
#include"IPC.h"
#include"masterP.h"

/** ------------------------------------------------------------------------ *
 * allowing not blocking getc, check if the user pressed enter
 *
 * PARAMETERS
 * (void)
 *
 * RETURN
 * returns 1 if enter has been pressed, 0 otherwise
 * ------------------------------------------------------------------------- */
int kbhit(void);

void masterP(Block_pos* segptr_pos, int* segptr_exit, int sid_getpos,
              int nb_blocks, int block_color[])
{

  struct sembuf unlock_acc;
  unlock_acc.sem_num = nb_blocks;
  unlock_acc.sem_op = 1;
  unlock_acc.sem_flg = 0;
   //{nb_blocks, 1, 0};
  struct sembuf wait_blocks[nb_blocks+1];
    for(int i = 0; i < nb_blocks+1; i++){
      wait_blocks[i].sem_num = i;
      wait_blocks[i].sem_op = -1;
      wait_blocks[i].sem_flg = 0;
    }


  int table_of_pixels[SIZE_X][SIZE_Y];

  //Initializes SDL and the colours
  init_output();
  printf("initialized\n");

  //update board
  for(int i = 0; i < SIZE_X; ++i){
    for(int j = 0; j < SIZE_Y; ++j)
      table_of_pixels[i][j] = 0;
  }

  for(int i = 0; i < nb_blocks; i++){
    for(int j = 0; j < SQUARE_WIDTH; j++){
      for(int k = 0; k < SQUARE_WIDTH; k++)
        table_of_pixels[segptr_pos[i].x+j][segptr_pos[i].y+k] = block_color[i];
    }
  }

  update_output(table_of_pixels);
  printf("Window opened..\n");

  //let block begin calculate
  if((semop(sid_getpos, wait_blocks, nb_blocks+1) == -1)) // pas grave si dernier élément inutile?
    perror("semop MP l.64");
  if((semop(sid_getpos, &unlock_acc, 1) == -1))
    perror("semop");

  //loop until key is pressed
  while((*segptr_exit) == 0){

    for(int i =0; i < nb_blocks+1; i++)
      printf("état sém %d: %d\n", i, semctl(sid_getpos, i, GETVAL, 0));

    //wait for blocks to detect out of bounds and calculate new positions
    if((semop(sid_getpos, wait_blocks, nb_blocks+1) == -1)) // pas grave si dernier élément inutile?
      perror("semop MP l.73");
    if((semop(sid_getpos, &unlock_acc, 1) == -1))
      perror("semop");
    printf("get trough.. 1 out of bound corrected\n"); //-------------------------------------------
    for(int i =0; i < nb_blocks+1; i++)
      printf("état sém %d: %d\n", i, semctl(sid_getpos, i, GETVAL, 0));


    //wait for blocks to detect overlaps and send message to each other
    if((semop(sid_getpos, wait_blocks, nb_blocks+1) == -1))
      perror("semop MP l.79");
    if((semop(sid_getpos, &unlock_acc, 1) == -1))
      perror("semop");
    printf("get trough.. 2 message sended\n"); //-------------------------------------------
    for(int i =0; i < nb_blocks+1; i++)
      printf("état sém %d: %d\n", i, semctl(sid_getpos, i, GETVAL, 0));


    //wait for blocks to receive msg and correct overlaps
    //then acquire lock on shm_pos
    if((semop(sid_getpos, wait_blocks, nb_blocks+1) == -1))
      perror("semop MP l.85");
    printf("get trough.. 3 corrected overlaps\n"); //-------------------------------------------
    for(int i =0; i < nb_blocks+1; i++)
      printf("état sém %d: %d\n", i, semctl(sid_getpos, i, GETVAL, 0));

    printf("updating board..\n");

    //update board..
    for(int i = 0; i < SIZE_X; ++i){
      for(int j = 0; j < SIZE_Y; ++j)
        table_of_pixels[i][j] = 0;
    }

    for(int i = 0; i < nb_blocks; i++){
      for(int j = 0; j < SQUARE_WIDTH; j++){
        for(int k = 0; k < SQUARE_WIDTH; k++)
          table_of_pixels[segptr_pos[i].x+j][segptr_pos[i].y+k] = block_color[i];
      }
    }
    //allows blocks to change shm_pos
    if((semop(sid_getpos, &unlock_acc, 1) == -1))
      perror("semop");

    //Apply the change on SDL display
    update_output(table_of_pixels);

    //won't let blocks work anymore if key pressed
    if(kbhit())
      (*segptr_exit) = 1;

    usleep(15000);
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
  if(ch != EOF && ch == '\n'){
  	//Put back the character on the input stream
    ungetc(ch, stdin);
    return 1;
  }

  return 0;
}
