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
#include <sys/msg.h>
#include <stdbool.h>
#include <errno.h>

//Including user libraries
#include "process_communication.h"


/**********************************************************************************
* Returns the number of members in a semaphore
**********************************************************************************/
unsigned short get_member_count(int sid){
        union semun semopts;
        struct semid_ds mysemds;

        semopts.buf = &mysemds;
        return(semopts.buf->sem_nsems);
}


/**********************************************************************************
*Locks semaphore 'member' with in sempahore set with id "sid"
**********************************************************************************/
void locksem(int sid, int member){

        struct sembuf sem_lock={0, -1, 0}; //IPC_NO_WAIT removed

        sem_lock.sem_num = member;
        
        if((semop(sid, &sem_lock, 1)) == -1){
                int errnum = errno;
                fprintf(stderr, "Lock failed (sem %d of set %d)\n",member,sid);
                fprintf(stderr, "Value of errno: %d\n",errno);
                fprintf(stderr, "Error: %s \n",strerror(errnum));
                exit(1);
        }
}


/**********************************************************************************
*Unlocks semaphore 'member' with in sempahore set with id "sid"
**********************************************************************************/
void unlocksem(int sid, int member){
        struct sembuf sem_unlock={member, 1,0}; //IPC_NO_WAIT removed

        sem_unlock.sem_num = member;

        /* Attempt to lock the semaphore set */
        if((semop(sid, &sem_unlock, 1)) == -1){
                int errnum = errno;
                fprintf(stderr, "Unlock failed (sem %d of set %d)\n",member,sid);
                fprintf(stderr, "Value of errno: %d)\n",errno);
                fprintf(stderr, "Error: %s \n",strerror(errnum));
                exit(1);
        }
}


/**********************************************************************************
*Creates a semaphore set with key "key", id "sid" and "members" members
**********************************************************************************/
void createsem(int *sid, key_t key, int members){

    if((*sid = semget(key, members, IPC_CREAT|IPC_EXCL|0666)) == -1) {
        fprintf(stderr, "Semaphore set already exists!\n");
        exit(1);
    }
}

/**********************************************************************************
*Creates a message queue set with key "key_q", id "msgqueue_id" and "members" members
**********************************************************************************/
void createqueue(int *msgqueue_id, key_t key_q, int members){
    /* Open the queue - create if necessary */
    if((*msgqueue_id = msgget(key_q, IPC_CREAT|IPC_EXCL|0660)) == -1) {
        perror("msgget");
        exit(1);
    }
}
        
/**********************************************************************************
* Returns the value of the "member" semaphore in the "sid" set
**********************************************************************************/
int getval(int sid, int member){

    int semval = semctl(sid, member, GETVAL, 0);
    return(semval);
}


/**********************************************************************************
* Set the value of the "member" semaphore in the "sid" set to "value"
**********************************************************************************/
void setval( int sid, int semnum, int value){
        union semun semopts;    

        semopts.val = value;
        semctl(sid, semnum, SETVAL, semopts);
}



/**********************************************************************************
* Set all the values of the semaphore set "sid" to "value"
**********************************************************************************/
void setall(int sid,int value){
    union semun semopts;
    semopts.val = value;

    int members = get_member_count(sid);
    for(int cntr=0; cntr<members; cntr++){
        semctl(sid,cntr, SETVAL, semopts);    
    }   
}


/**********************************************************************************
* Write "value" at index "index" away from the adress pointed by "segptr"
**********************************************************************************/
void writeshm(point* segptr,int index, point value){
    segptr[index] = value;
}


/**********************************************************************************
* Read "value" at index "index" away from the adress pointed by "segptr"
**********************************************************************************/
point readshm(point* segptr, int index){
    return segptr[index];
}

/**********************************************************************************
*Send a message of type mymsgbuf to the message queue with id "qid"
**********************************************************************************/
void send_message(int qid, struct mymsgbuf *qbuf){

    int length = sizeof(struct mymsgbuf) - sizeof(long);
    if((msgsnd(qid, (struct mymsgbuf *)qbuf,length, 0)) ==-1){
        perror("msgsnd");
        exit(1);
    }
}

/**********************************************************************************
*Receive a message of type mymsgbuf from the message queue with id "qid"
**********************************************************************************/
void read_message(int qid, struct mymsgbuf *qbuf, long type){
    int length = sizeof(struct mymsgbuf) - sizeof(long);
    
   if((msgrcv(qid, (struct mymsgbuf *)qbuf, length, type, 0)) ==-1){
        perror("msgrcv");
        exit(1);
    } 
}


/**********************************************************************************
* Remove the semaphore identified by "semid"
**********************************************************************************/
void remove_sem(int semid){
    printf("Semaphore %d is being deleted.\n",semid);
    semctl(semid, 0, IPC_RMID, 0);
}


/**********************************************************************************
* Remove the shared memory segment identified by "shmid"
**********************************************************************************/
void remove_shm(int shmid){       
        printf("Shared memory segment %d is being deleted.\n",shmid);
        shmctl(shmid, IPC_RMID, 0);
}


/**********************************************************************************
* Remove the message queue identified by "qid"
**********************************************************************************/
void remove_queue(int qid){
        printf("Message queue %d is being deleted.\n",qid);
        msgctl(qid, IPC_RMID, 0);
}