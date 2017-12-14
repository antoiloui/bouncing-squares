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

#include "process_communication.h"


unsigned short get_member_count(int sid){
        union semun semopts;
        struct semid_ds mysemds;

        semopts.buf = &mysemds;

        /* Return number of members in the semaphore set */
        return(semopts.buf->sem_nsems);
}



//Locks semaphore 'member' with in sempahore set with id "sid"
void locksem(int sid, int member){

        struct sembuf sem_lock={0, -1, 0}; //IPC_NO_WAIT removed

        if( member<0 || member>(get_member_count(sid)-1)){
                fprintf(stderr, "semaphore member %d out of range\n", member);
                return;
        }

        /* Attempt to lock the semaphore set */
        if(!getval(sid, member)){
                fprintf(stderr, "Semaphore resources exhausted (no lock)!\n");
                exit(1);
        }
        
        sem_lock.sem_num = member;
        
        if((semop(sid, &sem_lock, 1)) == -1){
                fprintf(stderr, "Lock failed\n");
                exit(1);
        }
        else
                printf("Semaphore resources decremented by one (locked)\n");

        dispval(sid, member);
}

void unlocksem(int sid, int member){
        struct sembuf sem_unlock={member, 1,0}; //IPC_NO_WAIT removed

        if( member<0 || member>(get_member_count(sid)-1)){
                fprintf(stderr, "semaphore member %d out of range\n", member);
                return;
        }

        sem_unlock.sem_num = member;

        /* Attempt to lock the semaphore set */
        if((semop(sid, &sem_unlock, 1)) == -1){
                fprintf(stderr, "Unlock failed\n");
                exit(1);
        }
        else
                printf("Semaphore resources incremented by one (unlocked)\n");

        dispval(sid, member);
}





void createsem(int *sid, key_t key, int members){
        int cntr;
        union semun semopts;

        if(members > SEMMSL) {
                printf("Sorry, max number of semaphores in a set is %d\n",
                        SEMMSL);
                exit(1);
        }

        printf("Attempting to create new semaphore set with %d members\n",
                                members);

        if((*sid = semget(key, members, IPC_CREAT|IPC_EXCL|0666)) == -1) {
                fprintf(stderr, "Semaphore set already exists!\n");
                exit(1);
        }

        semopts.val = SEM_RESOURCE_MAX;
        
        /* Initialize all members (could be done with SETALL) */        
        for(cntr=0; cntr<members; cntr++)
                semctl(*sid, cntr, SETVAL, semopts);
}



int getval(int sid, int member){
        int semval;

        semval = semctl(sid, member, GETVAL, 0);
        return(semval);
}

void setval( int sid, int semnum, int value){
        union semun semopts;    

        semopts.val = value;
        semctl(sid, semnum, SETVAL, semopts);
}


void setall(int sid,ushort value){
    union semun semopts;
    int cntr;
    
    int members = get_member_count(sid);
    ushort myArray[members];
    
    for(cntr=0; cntr<members; cntr++){
        myArray[cntr] = value;
    }

    semopts.array = myArray;
    
    semctl(sid,semnum, SETALL, semopts);    
}


point writeshm(point* segptr,int index, int value){
        segptr[index] = value;
}

point readshm(point* segptr, int index){
    return segptr[index];
}

void removeshm(int shmid){
        shmctl(shmid, IPC_RMID, 0);
        printf("Shared memory segment marked for deletion\n");
}