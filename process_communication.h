#ifndef PROCESS_COMMUNICATION
#define PROCESS_COMMUNICATION 


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

******************************************************************************************/


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

struct point_t
{
    int x;
    int y;
};
typedef struct point_t point;



union semun { 
        int val;                        /* value for SETVAL */ 
        struct semid_ds *buf;                /* buffer for IPC_STAT, IPC_SET */ 
        unsigned short int *array;         /* array for GETALL, SETALL */ 
        struct seminfo *__buf;                /* buffer for IPC_INFO */ 
}; 




//Locks semaphore 'member' with in sempahore set with id "sid"
unsigned short get_member_count(int sid);
void locksem(int sid, int member);
void unlocksem(int sid, int member);
unsigned short get_member_count(int sid);
void createsem(int *sid, key_t key, int members);
int getval(int sid, int member);
void setAll(int sid,ushort value);
void setval(int sid, int semnum, int value);
void writeshm(int shmid, point *segptr, char *text);
//void changemode(int shmid, char *mode); 
void removeshm(int shmid);
void readshm(point* segptr,int index);

#endif
