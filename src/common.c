#include "../headers/common.h"




void P (int semaphore) {
    struct sembuf op;
    op.sem_flg = 0;
    op.sem_num = 0;
    op.sem_op = -1;

    semop(semaphore, &op, 1);
}




void V (int semaphore) {
    struct sembuf op;
    op.sem_flg = 0;
    op.sem_num = 0;
    op.sem_op = 1;

    semop(semaphore, &op, 1);
}




void Z (int semaphore){
    struct sembuf op;

    op.sem_num = 0;
    op.sem_flg = 0;
    op.sem_op = 0;
   
    semop(semaphore, &op, 1);
} 

