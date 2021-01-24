#include "../headers/common.h"

int load(){
    FILE * fp;
    char key[20];
    int i;

    fp = fopen(CONFIG, "r");
    if(fp == NULL) return 1; 

    config = (Config *) malloc(sizeof(Config));
    if(config == NULL) return 1;

    fscanf(fp, "%s %d\n", key, &config->SO_TAXI);
    fscanf(fp, "%s %d\n", key, &config->SO_SOURCES);
    fscanf(fp, "%s %d\n", key, &config->SO_HOLES);
    fscanf(fp, "%s %d\n", key, &config->SO_TOP_CELLS);
    fscanf(fp, "%s %d\n", key, &config->SO_CAP_MIN);
    fscanf(fp, "%s %d\n", key, &config->SO_CAP_MAX);
    fscanf(fp, "%s %d\n", key, &config->SO_TIMENSEC_MIN);
    fscanf(fp, "%s %d\n", key, &config->SO_TIMENSEC_MAX);
    fscanf(fp, "%s %d\n", key, &config->SO_TIMEOUT);
    fscanf(fp, "%s %d\n", key, &config->SO_DURATION);

    fclose(fp);

    /* creazione array ARGS per passare info ai processi creati */
    args = malloc(NARGS * sizeof(char *));
    for(i = 0; i < NARGS - 1; i++) args[i] = malloc(sizeof(int));
    args[NARGS - 1] = NULL;

    /* crazione semaforo per sincronizzare la partenza della sim */
    sync_semaphore_id = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    TEST_ERROR;
    semctl(sync_semaphore_id, 0, SETVAL, config->SO_SOURCES + config->SO_TAXI); 

    return 0;
}

void init_world () {
    int i, j;

    map_id = shmget(IPC_PRIVATE, sizeof(Cell) * SO_WIDTH*SO_HEIGHT, S_IRUSR | S_IWUSR);
    TEST_ERROR;

    map = (Cell**) shmat(map_id, NULL, 0);
    TEST_ERROR;

    for(i = 0; i < SO_HEIGHT; i++) {
        for(j = 0; j < SO_WIDTH; j++) {
            map[i][j].is_hole = 0;
            map[i][j].source_pid = 0;
            map[i][j].req_access_sem = 0; 
            map[i][j].cap_semid = 0; 
            map[i][j].cell_cap = config->SO_CAP_MIN + rand() % (config->SO_CAP_MAX - config->SO_CAP_MIN +1);
            map[i][j].travel_time = config->SO_TIMENSEC_MIN + rand() % (config->SO_TIMENSEC_MAX - config->SO_TIMENSEC_MIN + 1);
            map[i][j].traffic = 0;
        }
    }

    gen_holes();

    for(i = 0; i < SO_HEIGHT; i++) {
        for(j = 0; j < SO_WIDTH; j++) {
            if(!map[i][j].is_hole) {
                map[i][j].cap_semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600); /* semaforo della capienza */
                TEST_ERROR;
                semctl(map[i][j].cap_semid, 0, SETVAL, map[i][j].cell_cap); 
            }
        }
    }

    gen_sources();

    for(i = 0; i < SO_HEIGHT; i++) {
        for(j = 0; j < SO_WIDTH; j++) {
            if(map[i][j].source_pid > 0) {
                map[i][j].req_access_sem = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600); /* semaforo per l'accesso alla queue */
                TEST_ERROR;
                semctl(map[i][j].req_access_sem, 0, SETVAL, 1); 

                pipe(map[i][j].req_pipe); /* pipe delle richieste */
            }
        }
    }


}

void gen_holes() {
    int i, j; 
    int count; 

    count = 0; 
    while (count < config->SO_HOLES) {
        i = rand() % SO_HEIGHT;
        j = rand() % SO_WIDTH;

        if (check_hole(i, j)) {
            map[i][j].is_hole = 1; 
            count++;
        }
    }
}

int check_hole(int x, int y){
    int i, j;

    for(i = -1; i < 3; i++) {
        for(j = -1; j < 3; j++) {
            if ((y + i >= 0 && y + i < SO_HEIGHT) && (x + j >= 0 && x + j < SO_WIDTH) && map[y + i][x + j].is_hole == 1){
                return 0;
            }   
        }
    }

    return 1;
}



void gen_sources () {
    int count, child_pid;
    char ** args;
    int i, j;
    

    count = 0;
    while(count < config->SO_SOURCES) {

        i = rand() % SO_HEIGHT;
        j = rand() % SO_WIDTH;

        if(!map[i][j].is_hole && !map[i][j].source_pid) {
        
            switch(child_pid = fork()) {
                case -1:
                    TEST_ERROR;
                    exit(EXIT_FAILURE);
                    break;

                case 0:
                    /* args = malloc((4) * sizeof(char *)); */

                    /* args[0] = malloc(sizeof(int)); */
                    sprintf(args[0],"%d", map_id);

                    /* args[1] = malloc(sizeof(int)); */
                    sprintf(args[1],"%d", i);

                    /* args[2] = malloc(sizeof(int)); */
                    sprintf(args[2],"%d", j);

                    /* args[3] = NULL; */

                    execvp("./source", args);

                    break;

                default:
                    map[i][j].source_pid = child_pid;
                    if(count == 0) source_gpid = child_pid; /* set first source as group leader */
                    setpgid(child_pid, source_gpid); /* set all sources in the same group */
                    break;
            }

            count++;
        }
    }
}


void gen_taxi () {

}

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

void sync_simulation(int semid, int nsem, int value){
    struct sembuf sop;

    sop.sem_num = nsem;
    sop.sem_flg = 0;
    sop.sem_op = value;

    while(semop(semid,&sop,1) == -1) {
        if(errno != EINTR){
            TEST_ERROR;
        }
    } 
}

void unload () {
    int i, j;

    /* 1. dealloco args */
    for(i = 0; i < NARGS; i++) free(args[i]);
    free(args);

    /* 2. rimuovo i semafori */
    for(i = 0; i < SO_HEIGHT; i++) {
        for(j = 0; j < SO_WIDTH; j++) {
            if(!map[i][j].is_hole) semctl(map[i][j].cap_semid, 0, IPC_RMID);
            if(map[i][j].source_pid > 0) semctl(map[i][j].req_access_sem, 0, IPC_RMID);
        }
    }

    semctl(sync_semaphore_id, 0, IPC_RMID);

    /* 3. detach & deallocazione map */
    shmdt(map);
    TEST_ERROR;

    shmctl(map_id, IPC_RMID, NULL);
    TEST_ERROR;
}

void print_map () {
    int i, j;
    char s[4];
    int traffic;

    printf("\n+");

    for (j = 0; j < SO_WIDTH; j++) printf("---+");

    for(i = 0; i < SO_HEIGHT; i++){
        printf("\n|");
        for(j = 0; j < SO_WIDTH; j++){

            switch(map[i][j].is_hole) {
                case 1: printf(HOLE "   " ENDSTYLE); break;

                default:
                    if (map[i][j].source_pid > 0) printf(SOURCE);
                    if(!semctl(map[i][j].cap_semid, 0, GETVAL)) printf(BUSY);
                    traffic = map[i][j].cell_cap - semctl(map[i][j].cap_semid, 0, GETVAL);
                    sprintf(s,"%2d ", traffic);
                    printf("%s" ENDSTYLE, traffic == 0 ? "   " : s);
            }

            printf("|");
        }
    
        printf("\n+");
        for (j = 0; j < SO_WIDTH; j++){ printf("---+"); }
    }
    printf("\n\n");


}