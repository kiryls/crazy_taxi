#include "../headers/common.h"

/* 
################################################################################################# 
                                    SEMAPHORE HANDLING
descrizione
################################################################################################# 
*/
void P (int semaphore) {
    struct sembuf op;
    op.sem_flg = 0;
    op.sem_num = 0;
    op.sem_op = -1;

    semop(semaphore, &op, 1);
    /* printf("P(%d): %d\n", semaphore, getpid()); */
}

void V (int semaphore) {
    struct sembuf op;
    op.sem_flg = 0;
    op.sem_num = 0;
    op.sem_op = 1;

    semop(semaphore, &op, 1);
    /* printf("V(%d): %d\n", semaphore, getpid()); */
}

/* 
################################################################################################# 
                                        SIMULATION SYNC
descrizione
################################################################################################# 
*/
void Z (int semaphore){
    struct sembuf op;

    op.sem_num = 0;
    op.sem_flg = 0;
    op.sem_op = 0;
   
    /* while(semop(semaphore, &sop, 1) == -1 && errno != EINTR) TEST_ERROR; */
    semop(semaphore, &op, 1);
} 

