#include "../headers/common.h"

/* 
################################################################################################# 
                                    SEMAPHORE HANDLING
descrizione
################################################################################################# 
*/
void P (int semaphore, int index) {
    struct sembuf operation;

    operation.sem_flg = 0;
    operation.sem_num = index;
    operation.sem_op = -1;

    semop(semaphore, &operation, 1);
}

void V (int semaphore, int index) {
    struct sembuf operation;

    operation.sem_flg = 0;
    operation.sem_num = index;
    operation.sem_op = 1;

    semop(semaphore, &operation, 1);
}

/* 
################################################################################################# 
                                        SIMULATION SYNC
descrizione
################################################################################################# 
*/
void ALLSET (int semid, int nsem, int value){
    struct sembuf sop;

    sop.sem_num = nsem;
    sop.sem_flg = 0;
    sop.sem_op = value;
   
    while(semop(semid,&sop,1) == -1 && errno != EINTR) TEST_ERROR;
} 

