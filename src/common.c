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

    /* semaforo sync creazione sorgenti */
    sync_sources_sem = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    /* TEST_ERROR; */
    semctl(sync_sources_sem, 0, SETVAL, config->SO_SOURCES); 

    /* semaforo sync creazione taxi */
    sync_taxi_sem = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    /* TEST_ERROR; */
    semctl(sync_taxi_sem, 0, SETVAL, config->SO_SOURCES); 

    return 0;
}

void init_world () {
    int i, j;

    map_id = shmget(IPC_PRIVATE, sizeof(int) * SO_HEIGHT, /* S_IRUSR | S_IWUSR */ IPC_CREAT | 0600);
    map_row_ids = shmat(map_id, NULL, 0);

    for(i = 0; i < SO_HEIGHT; i++) {
        map_row_ids[i] = shmget(IPC_PRIVATE, sizeof(Cell) * SO_WIDTH, IPC_CREAT | 0600);
        map[i] = shmat(map_row_ids[i], NULL, 0);
    }
    

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
                /* TEST_ERROR; */
                semctl(map[i][j].cap_semid, 0, SETVAL, map[i][j].cell_cap); 
            }
        }
    }

    gen_sources();
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

int check_hole(int r, int c){
    int i, j;

    for(i = -1; i < 3; i++) {
        for(j = -1; j < 3; j++) {
            if ((r + i >= 0 && r + i < SO_HEIGHT) && (c + j >= 0 && c + j < SO_WIDTH) && map[r + i][c + j].is_hole){
                return 0;
            }   
        }
    }

    return 1;
}



void gen_sources () {
    int count, child_pid;
    int i, j;
    int exec_ret;
    
    count = 0;
    while(count < config->SO_SOURCES) {

        i = rand() % SO_HEIGHT;
        j = rand() % SO_WIDTH;

        if(!map[i][j].is_hole && !map[i][j].source_pid) {

            pipe(map[i][j].req_pipe);
        
            switch(child_pid = fork()) {
                case -1:
                    TEST_ERROR;
                    exit(EXIT_FAILURE);
                    break;

                case 0:
                    sprintf(args[0],"%d", map_id);
                    sprintf(args[1],"%d", i);
                    sprintf(args[2],"%d", j);
                    sprintf(args[3],"%d", sync_sources_sem);
                    exec_ret = execvp("./out/source", args);
                    fprintf(stderr, "errore nella exec delle sources: %d\n", exec_ret);
                    break;

                default:
                    close(map[i][j].req_pipe[R]);
                    close(map[i][j].req_pipe[W]);

                    if(count == 0) source_gpid = child_pid; /* set first source as group leader */
                    setpgid(child_pid, source_gpid); /* set all sources in the same group */
                    break;
            }

            count++;
        }
    }
}


void gen_taxi () {
    int i, j, count, child_pid;

    count = 0;
    while(count < config->SO_TAXI) {

        i = rand() % SO_HEIGHT;
        j = rand() % SO_WIDTH;

        if(semctl(map[i][j].cap_semid, 0, GETVAL) > 0 && !map[i][j].is_hole) {
            child_pid = gen_one_taxi(i, j);
            if(count == 0) taxi_gpid = child_pid;
            setpgid(child_pid, taxi_gpid);
            count++;
        }

    }
}

int gen_one_taxi (int i, int j) {
    int child_pid, exec_ret;

    P(map[i][j].cap_semid, 0);

    switch(child_pid = fork()) {
        case -1:
            TEST_ERROR;
            exit(EXIT_FAILURE);

        case 0:
            sprintf(args[0], "%d", map_id);
            sprintf(args[1], "%d", i);
            sprintf(args[2], "%d", j);
            exec_ret = execvp("./out/taxi", args);
            fprintf(stderr, "errore nella exec dei taxi: %d\n", exec_ret);
            break;

        default:
            return child_pid;
    }
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
    int ret;

    /* 1. args */
    for(i = 0; i < NARGS; i++) free(args[i]);
    free(args);

    /* 2. semafori & pipes */
    for(i = 0; i < SO_HEIGHT; i++) {
        for(j = 0; j < SO_WIDTH; j++) {
            if(!map[i][j].is_hole) semctl(map[i][j].cap_semid, 0, IPC_RMID);
            if(map[i][j].source_pid > 0) semctl(map[i][j].req_access_sem, 0, IPC_RMID);  
        }
    }

    semctl(sync_sources_sem, 0, IPC_RMID);

    /* 3. map */
    for(i = 0; i < SO_HEIGHT; i++) {
        if(shmdt(map[i])) TEST_ERROR;
        if(shmctl(map_row_ids[i], IPC_RMID, NULL)) TEST_ERROR;
    }
    
    if (shmdt(map_row_ids)) TEST_ERROR;
    if(shmctl(map_id, IPC_RMID, NULL)) TEST_ERROR;
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