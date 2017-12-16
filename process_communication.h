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
#include <time.h>
#include<stdbool.h>


/************************************************************************************

// One semaphore structure for each semaphore in the system. 
struct sem {
        short   sempid;         // pid of last operation 
        ushort  semval;         // current value 
        ushort  semncnt;        // num procs awaiting increase in semval 
        ushort  semzcnt;        // num procs awaiting semval = 0 
};

// semop system call takes an array of these 
struct sembuf {
    ushort  sem_num;        // semaphore index in array 
    short   sem_op;         // semaphore operation 
    short   sem_flg;        // operation flags 
};


union semun {
    int              val;    // Valeur pour SETVAL 
    struct semid_ds *buf;    // Tampon pour IPC_STAT, IPC_SET
    unsigned short  *array;  // Tableau pour GETALL, SETALL
    struct seminfo  *__buf;  // Tampon pour IPC_INFO
                             //   (spécifique à Linux) 
};

// arg for semctl system calls. 
union semun {
        int val;                // value for SETVAL 
        struct semid_ds *buf;   // buffer for IPC_STAT & IPC_SET 
        ushort *array;          // array for GETALL & SETALL 
        struct seminfo *__buf;  // buffer for IPC_INFO 
        void *__pad;
};


/* message buffer for msgsnd and msgrcv calls 
struct msgbuf {
    long mtype;         // type of message 
    char mtext[1];      // message text 
};
*/
/*******************************************************************************************/

/**********************************************************************************
*
**********************************************************************************/
struct square_t
{
  int x;
  int y;
  int color;
  int speedx;
  int speedy;
};

typedef struct square_t square;


/**********************************************************************************
*
**********************************************************************************/
struct point_t
{
    int x;
    int y;
};

typedef struct point_t point;


/**********************************************************************************
*
**********************************************************************************/
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

    long receiver; // type
    int sender; 
    
    struct speed_s speed;      // message text 

};





/**********************************************************************************
*
**********************************************************************************/
unsigned short get_member_count(int sid);

/**********************************************************************************
*
**********************************************************************************/
void locksem(int sid, int member);

/**********************************************************************************
*
**********************************************************************************/
void unlocksem(int sid, int member);

/**********************************************************************************
*
**********************************************************************************/
void createsem(int *sid, key_t key, int members);

/**********************************************************************************
*
**********************************************************************************/
void createqueue(int *msgqueue_id, key_t key_q, int members);
/**********************************************************************************
*
**********************************************************************************/
void send_message(int qid, struct mymsgbuf *qbuf, long receiver);
/**********************************************************************************
*
**********************************************************************************/
void read_message(int qid, struct mymsgbuf *qbuf, long receiver);

/**********************************************************************************
*
**********************************************************************************/
int getval(int sid, int member);


/**********************************************************************************
*
**********************************************************************************/
void setall(int sid, int value);
/**********************************************************************************
*
**********************************************************************************/
void setval(int sid, int semnum, int value);

/**********************************************************************************
*
**********************************************************************************/
void writeshm(point* segptr, int shmid, point value);

/**********************************************************************************
*
**********************************************************************************/
point readshm(point* segptr,int index);

/**********************************************************************************
*
**********************************************************************************/
//void changemode(int shmid, char *mode); 

/**********************************************************************************
*
**********************************************************************************/
void remove_shm(int shmid);

/**********************************************************************************
*
**********************************************************************************/
void remove_sem(int semid);

/**********************************************************************************
*
**********************************************************************************/
void remove_queue(int qid);


#endif
