#include "../headers/common.h"
#include "../headers/master.h"




int main(int argc, char *argv[]) {    
    srand(time(NULL));

    if (load()) exit(EXIT_FAILURE);
        fprintf(logfile, "+ loading done\n");

    init_world();
        fprintf(logfile, "+ world created\n");

    gen_sources();
        fprintf(logfile, "+ sources generated\n");

    gen_taxi();
        fprintf(logfile, "+ taxi generated +++\n");

    gen_timer();
        fprintf(logfile, "+ timer (%d) set\n", timer_id);

    set_signals();
        fprintf(logfile, "+ signals set\n\n");

        printf("\t\t\t~~~ S T A R T ~~~ \n");
    simulate();
        printf("\t\t\t ~~~  E  N  D  ~~~ \n");

    aftermath();
    
    unload();
        fprintf(logfile, "*** unloaded all structures ***\n");

    exit(EXIT_SUCCESS);
}




int load(){
    char key[20];
    int i;

    settings = fopen(CONFIG, "r");
    if(settings == NULL) return 1; 
    
    config = (Config *) malloc(sizeof(Config));
    if(config == NULL) return 1;

    fscanf(settings, "%s %d\n", key, &config->SO_TAXI);
    fscanf(settings, "%s %d\n", key, &config->SO_SOURCES);
    fscanf(settings, "%s %d\n", key, &config->SO_HOLES);
    fscanf(settings, "%s %d\n", key, &config->SO_TOP_CELLS);
    fscanf(settings, "%s %d\n", key, &config->SO_CAP_MIN);
    fscanf(settings, "%s %d\n", key, &config->SO_CAP_MAX);
    fscanf(settings, "%s %d\n", key, &config->SO_TIMENSEC_MIN);
    fscanf(settings, "%s %d\n", key, &config->SO_TIMENSEC_MAX);
    fscanf(settings, "%s %d\n", key, &config->SO_TIMEOUT);
    fscanf(settings, "%s %d\n", key, &config->SO_DURATION);

    fclose(settings);

    logfile = fopen("./logs/master.log", "w");
    if(logfile == NULL) return 1;

    /* creazione array ARGS per passare info ai processi creati */
    args = malloc(NARGS * sizeof(char *));
    for(i = 0; i < NARGS - 1; i++) args[i] = malloc(sizeof(int));
    args[NARGS - 1] = NULL;

    sync_all = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    semctl(sync_all, 0, SETVAL, 1);

    ledger_id = shmget(IPC_PRIVATE, sizeof(Ledger), IPC_CREAT | 0600);
    ledger = shmat(ledger_id, NULL, 0);

    ledger->source_section = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    ledger->taxi_section = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);

    semctl(ledger->source_section, 0, SETVAL, 1);
    semctl(ledger->taxi_section, 0, SETVAL, 1);

    tops = calloc(config->SO_TOP_CELLS, sizeof(Top));

    return 0;
}




void unload () {
    int i, j;
    int ret;

    /* 1. args */
    for(i = 0; i < NARGS; i++) free(args[i]);
    free(args);

    /* 2. semafori map */
    for(i = 0; i < SO_HEIGHT; i++) {
        for(j = 0; j < SO_WIDTH; j++) {
            if(!map[i][j].is_hole) semctl(map[i][j].cap_semid, 0, IPC_RMID);
            if(map[i][j].source_pid > 0) semctl(map[i][j].update_traffic_sem, 0, IPC_RMID);  
        }
    }

    /* 3. map */
    for(i = 0; i < SO_HEIGHT; i++) {
        if(shmdt(map[i])) TEST_ERROR;
        if(shmctl(map_row_ids[i], IPC_RMID, NULL)) TEST_ERROR;
    }
    
    if(shmdt(map_row_ids)) TEST_ERROR;
    if(shmctl(map_id, IPC_RMID, NULL)) TEST_ERROR;

    /* 4. sync all */
    semctl(sync_all, 0, IPC_RMID);

    /* 5. ledger */
    semctl(ledger->source_section, 0, IPC_RMID);
    semctl(ledger->taxi_section, 0, IPC_RMID);

    if(shmdt(ledger)) TEST_ERROR;
    if(shmctl(ledger_id, IPC_RMID, NULL)) TEST_ERROR;

    /* 6. tops */
    free(tops);
}




void init_world () {
    int i, j;

    for(i = 0; i < config->SO_TOP_CELLS; i++) tops[i].val = -1;

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
            map[i][j].cap_semid = 0; 
            map[i][j].cell_cap = config->SO_CAP_MIN + rand() % (config->SO_CAP_MAX - config->SO_CAP_MIN +1);
            map[i][j].travel_time = config->SO_TIMENSEC_MIN + rand() % (config->SO_TIMENSEC_MAX - config->SO_TIMENSEC_MIN + 1);
            map[i][j].update_traffic_sem = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600); 
            semctl(map[i][j].update_traffic_sem, 0, SETVAL, 1);
            map[i][j].traffic = 0;
        }
    }

    gen_holes();

    for(i = 0; i < SO_HEIGHT; i++) {
        for(j = 0; j < SO_WIDTH; j++) {
            if(!map[i][j].is_hole) {
                map[i][j].cap_semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600); 
                /* TEST_ERROR; */
                semctl(map[i][j].cap_semid, 0, SETVAL, map[i][j].cell_cap); 
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




void gen_timer () {
    int exec_ret;
    sprintf(args[0], "%d", config->SO_DURATION);
    sprintf(args[1], "%d", sync_all);

    switch(timer_id = fork()) {
        case -1:
            TEST_ERROR;
            exit(EXIT_FAILURE);
            break;

        case 0:
            exec_ret = execvp("./out/timer", args);
            fprintf(stderr, "||| errore nella exec del timer: %d |||\n", exec_ret);
            break;

        default:
            break;
    }
}




void gen_sources () {
    int count, child_pid;
    int i, j;
    int exec_ret;
    
    count = 0;
    while(count < config->SO_SOURCES) {

        i = rand() % SO_HEIGHT;
        j = rand() % SO_WIDTH;

        if(map[i][j].is_hole == 0 && map[i][j].source_pid == 0) {

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
                    sprintf(args[3],"%d", sync_all);
                    sprintf(args[4], "%d", ledger_id);

                    exec_ret = execvp("./out/source", args);
                    fprintf(stderr, "||| errore nella exec delle sources: %d |||\n", exec_ret);
                    break;

                default:
                    map[i][j].source_pid = child_pid;
                    close(map[i][j].req_pipe[W]);

                    if(count == 0) child_gpid = child_pid; /* set first source as child group leader */
                    setpgid(child_pid, child_gpid);
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
            count++;
        }

    }
}




int gen_one_taxi (int i, int j) {
    int child_pid, exec_ret;

    P(map[i][j].cap_semid);

    switch(child_pid = fork()) {
        
        case -1:
            TEST_ERROR;
            exit(EXIT_FAILURE);

        case 0:
            setpgid(getpid(), child_gpid);

            sprintf(args[0], "%d", map_id);
            sprintf(args[1], "%d", i);
            sprintf(args[2], "%d", j);
            sprintf(args[3], "%d", config->SO_TIMEOUT);
            sprintf(args[4], "%d", sync_all);
            sprintf(args[5], "%d", ledger_id);

            exec_ret = execvp("./out/taxi", args);
            fprintf(stderr, "||| errore nella exec dei taxi: %d |||\n", exec_ret);
            break;

        default:
            return child_pid;
    }
}




void respawn () {
    int i, j, child;

    do {
        i = rand() % SO_HEIGHT;
        j = rand() % SO_WIDTH;
    } while (map[i][j].is_hole || semctl(map[i][j].cap_semid, 0, GETVAL) == 0);

    child = gen_one_taxi(i, j);
}




void simulate () {
    int i, j;
    int child;
    int status;

    P(sync_all);

    while((child = wait(&status)) > 0 && semctl(sync_all, 0, GETVAL) == 0)
        if(WEXITSTATUS(status) == TAXI_ABORT) respawn();
    
    if(killpg(child_gpid, SIGTERM) < 0) TEST_ERROR;
    
    P(sync_all);

    while((child = wait(&status)) > 0) {

        switch (WEXITSTATUS(status)) {
            case TAXI_ABORT: 
                fprintf(logfile, "- taxi %d aborted\n", child);
                break;

            case TAXI_EXIT: 
                fprintf(logfile, "- taxi %d exited\n", child);
                break;

            case TIMER_EXIT:
                fprintf(logfile, "- timer %d exited\n", child);
                break;

            case SOURCE_EXIT: 
                fprintf(logfile, "- source %d exited\n", child);
                break;

            case EXIT_SUCCESS:
                fprintf(logfile, "- child %d terminated\n", child);
                break;
            default:
                fprintf(logfile, "- child %d exited with %d\n", child, WEXITSTATUS(status));
        }
    }

    if(errno == ECHILD) fprintf(logfile, "\n*** all children terminated successfully ***\n"); 

}




void select_tops() {
    int i, j, count;

    count = 0;
    while(count < config->SO_TOP_CELLS) {

        for(i = 0; i < SO_HEIGHT; i++) {
            for(j = 0; j < SO_WIDTH; j++) {
                if(map[i][j].traffic > tops[count].val && !is_top(i, j)) {
                    tops[count].p.r = i;
                    tops[count].p.c = j;
                    tops[count].val = map[i][j].traffic;
                }
            }
        } 

        count++; 
    }
    
}




int is_top(int r, int c) {
    int i;

    for(i = 0; i < config->SO_TOP_CELLS && tops[i].val != -1; i++) {
        if(tops[i].p.r == r && tops[i].p.c == c) return 1;
    }

    return 0;
}




void set_signals() {
    struct sigaction sa;
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);

    sa.sa_flags = SA_RESTART;
    sa.sa_mask = mask;
    sa.sa_handler = print_map_handler;

    sigaction(SIGALRM,&sa,NULL);

    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);

    sa.sa_flags = 0;
    sa.sa_mask = mask;
    sa.sa_handler = wrap_up;
    sigaction(SIGTERM,&sa,NULL);
    sigaction(SIGINT, &sa, NULL);
}




void print_map_handler(int sig) {
    print_map(); 
}




void wrap_up(int sig) {
    V(sync_all);
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




void aftermath () {
    int i, j;
    char s[4];
    int traffic;

    select_tops();

    printf("\n+");

    for (j = 0; j < SO_WIDTH; j++) printf("---+");

    for(i = 0; i < SO_HEIGHT; i++){
        printf("\n|");
        for(j = 0; j < SO_WIDTH; j++){

            switch(map[i][j].is_hole) {
                case 1: printf(HOLE "   " ENDSTYLE); break;

                default:
                    if (map[i][j].source_pid > 0) printf(SOURCE);
                    if(is_top(i, j)) printf(BUSY);
                    traffic = map[i][j].traffic; 
                    sprintf(s,"%2d ", traffic);
                    printf("%s" ENDSTYLE, traffic == 0 ? "   " : s);
            }

            printf("|");
        }
    
        printf("\n+");
        for (j = 0; j < SO_WIDTH; j++){ printf("---+"); }
    }
    printf("\n\n");
    printf("(( (  (   (    AFTERMATH    )   )  ) ))\n\n");
    printf("General stats:\n");
    printf("\tTot requests served:   %d\n", ledger->completed_rides);
    printf("\tTot failed requests:   %d\n", ledger->failed_rides);
    printf("\tTot unfilled requests: %d\n", ledger->tot_requests - (ledger->completed_rides + ledger->failed_rides));
    printf("\t\t~ ~ ~ ~\n");

    printf("\tLongest distance taxi:               #%d which crossed %d cells\n", ledger->best_distance.taxi_id, ledger->best_distance.tot_length);
    printf("\tTaxi that served the longest task:   #%d which took a task for %.3f seconds\n", ledger->best_longevity.taxi_id, ledger->best_longevity.time);
    printf("\tMost operative taxi:                 #%d which served %d rides\n", ledger->most_rides.taxi_id, ledger->most_rides.completed_rides);
}

