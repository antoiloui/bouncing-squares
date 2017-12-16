#include<stdio.h>
#include<stdlib.h>
#include<errno.h>

#include"constants.h"
#include"IPC.h"
#include"blockP.h"


static int hasIntersection(Block_pos* segptr_pos, int bid, int nb_blocks);

void blockP(Block_pos* segptr_pos, int* segptr_exit, int sid_getpos, int qid,
                              int bid, int nb_blocks, int speedx, int speedy)
{

  struct sembuf lock_acc;
  lock_acc.sem_num = nb_blocks;
  lock_acc.sem_op = -1;
  lock_acc.sem_flg = 0;

  struct sembuf unlock_acc;
  unlock_acc.sem_num = nb_blocks;
  unlock_acc.sem_op = 1;
  unlock_acc.sem_flg = 0;

  struct sembuf pos_ready;
  pos_ready.sem_num = bid;
  pos_ready.sem_op = 1;

  struct sembuf wait_0;
  wait_0.sem_num = bid;
  wait_0.sem_op = 0;
  wait_0.sem_flg = 0;


  //wait 0 from masterP to let block work
  if((semop(sid_getpos, &wait_0, 1) == -1)){
    perror("BP semop (l.38)");
  }

  while((*segptr_exit) == 0){

    segptr_pos[bid].x += speedx;
    segptr_pos[bid].y += speedy;
    //Correcting out of bound
    //hit vertical wall?
    if(segptr_pos[bid].x > SIZE_X-SQUARE_WIDTH){
      speedx = -1;
      segptr_pos[bid].x = SIZE_X-SQUARE_WIDTH;
    }
    if(segptr_pos[bid].x < 0){
      speedx = 1;
      segptr_pos[bid].x = 0;
    }

    //hit horizontal wall?
    if(segptr_pos[bid].y > SIZE_Y-SQUARE_WIDTH){
      speedy = -1;
      segptr_pos[bid].y = SIZE_Y-SQUARE_WIDTH;
    }
    if(segptr_pos[bid].y < 0){
      speedy = 1;
      segptr_pos[bid].y = 0;
    }
    for(int i =0; i < nb_blocks+1; i++)
      printf("(P%d)état sém %d: %d\n", bid, i, semctl(sid_getpos, i, GETVAL, 0));
    //signals masterP this block has corrected out of bounds
    //and wait 0 from masterP to awake
    if((semop(sid_getpos, &pos_ready, 1) == -1)){
      perror("BP semop (l.69)");
    }
    if((semop(sid_getpos, &wait_0, 1) == -1)){
      perror("BP semop (l.38)");
    }

    int intersect = hasIntersection(segptr_pos, bid, nb_blocks);

    for(int i =0; i < nb_blocks+1; i++)
      printf("(P%d)état sém %d: %d\n", bid, i, semctl(sid_getpos, i, GETVAL, 0));

    struct mymsgbuf myspeed;
    myspeed.mtype = 10*intersect + bid;
    myspeed.mspeed.x = speedx;
    myspeed.mspeed.y = speedy;


    if(intersect != bid ){
      printf("(P%d)type sent:%d \n", bid, (int)myspeed.mtype);
      send_msg(qid, &myspeed);
    }
    //wait for all blocks to send speed if needed
    if((semop(sid_getpos, &pos_ready, 1) == -1)){
      perror("BP semop (l.87)");
    }
    if((semop(sid_getpos, &wait_0, 1) == -1)){
      perror("BP semop (l.38)");
    }

    if(intersect != bid){

      struct mymsgbuf new_speed;
      printf("ICI 1\n");
      long type = 10*bid + intersect;
      printf("(P%d) type expected:%d \n", bid, (int)type);
      read_msg(qid, type, &new_speed);
      printf("ICI 2\n");
      speedx = new_speed.mspeed.x;
      speedy = new_speed.mspeed.y;
      printf("ICI 3\n");
      if((semop(sid_getpos, &lock_acc, 1) == -1))
        perror("BP semop (l.100)");
      printf("Acces locked..\n");
      segptr_pos[bid].x += (2*speedx);
      segptr_pos[bid].y += (2*speedy);

      //let other blocks acces to shm_pos
      printf("Acces gotta unlocked..\n");
      if((semop(sid_getpos, &unlock_acc, 1) == -1))
        perror("BP semop (l. 107)");

    }

    //sem to 1 then wait it goes back to 0
    if((semop(sid_getpos, &pos_ready, 1) == -1)){
      perror("BP semop (l.113)");
    }
    if((semop(sid_getpos, &wait_0, 1) == -1)){
      perror("BP semop (l.38)");
    }
  }

}

//MUTEX!! only 1 block at a time can check if it has intersection and then swap direction
static int hasIntersection(Block_pos* segptr_pos, int bid, int nb_blocks){

  int tmp_inter = bid;
  Block_pos curr_b;
  curr_b.x = segptr_pos[bid].x;
  curr_b.y = segptr_pos[bid].y;

  for(int i = 0; i<nb_blocks; i++){
    Block_pos tmp_b;
    tmp_b.x = segptr_pos[i].x;
    tmp_b.y = segptr_pos[i].y;

    if(abs(curr_b.x-tmp_b.x)<SQUARE_WIDTH && abs(curr_b.y-tmp_b.y)<SQUARE_WIDTH
          && bid!=i)

      tmp_inter = i;
  }

  return tmp_inter;
}


// find sibling bid = 2: pid = (ppid + 1)  + 2???
