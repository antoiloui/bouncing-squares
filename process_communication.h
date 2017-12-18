#ifndef PROCESS_COMMUNICATION
#define PROCESS_COMMUNICATION 

//Including system libraries
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

struct square_t
{
  int x;
  int y;
  int color;
  int speedx;
  int speedy;
};

typedef struct square_t square;

struct point_t
{
    int x;
    int y;
};

typedef struct point_t point;

union semun { 
        int val;                            /* value for SETVAL */ 
        struct semid_ds *buf;              /* buffer for IPC_STAT, IPC_SET */ 
        unsigned short int *array;         /* array for GETALL, SETALL */ 
        struct seminfo *__buf;             /* buffer for IPC_INFO */ 
}; 


struct speed_s{
  int speed_x;
  int speed_y;
};

struct mymsgbuf {
    long type; 
    int sender; 
    
    struct speed_s speed;   
};

unsigned short get_member_count(int sid);
void locksem(int sid, int member);
void unlocksem(int sid, int member);
void createsem(int *sid, key_t key, int members);

void createqueue(int *msgqueue_id, key_t key_q, int members);
void send_message(int qid, struct mymsgbuf *qbuf);
void read_message(int qid, struct mymsgbuf *qbuf, long type);

int getval(int sid, int member);
void setall(int sid, int value);
void setval(int sid, int semnum, int value);

void writeshm(point* segptr, int shmid, point value);
point readshm(point* segptr,int index);

void remove_shm(int shmid);
void remove_sem(int semid);
void remove_queue(int qid);


#endif
