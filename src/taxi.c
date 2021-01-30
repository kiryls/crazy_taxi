#include "../headers/common.h"
#include "../headers/taxi.h"

/* 
################################################################################################# 
                                            MAIN
descrizione
################################################################################################# 
*/
int main(int argc, char const *argv[]) {
    
    int i, j;
    sigset_t alarm_mask;

    set_signals();
    
    init(argv);

    ALLSET(sync_all, 0, 0);
 
    fprintf(logp, "%d (%d,%d) req access sem = %d\n", getpid(), p.r+1, p.c+1, semctl(map[p.r][p.c].req_access_sem, 0, GETVAL));

    get_req();

    raise(SIGALRM);

    exit(EXIT_FAILURE);
}

/* 
################################################################################################# 
                                              INIT 
descrizione
################################################################################################# 
*/
void init (const char * argv[]) {
    int i;

    map_id = atoi(argv[0]);
    p.r = atoi(argv[1]);
    p.c = atoi(argv[2]);
    TIMEOUT = atoi(argv[3]);
    sync_all = atoi(argv[4]);



    rep.tot_length = 0;
    rep.tot_time = 0;
    rep.completed_rides = 0;

    map_row_ids = (int*) shmat(map_id, NULL, 0);

    for(i = 0; i < SO_HEIGHT; i++) 
        map[i] = shmat(map_row_ids[i], NULL, 0);

    logp = fopen("/home/kiryls/Documents/Coding/project/logs/taxi.log", "a");

    write_log(logp);
}

/* 
################################################################################################# 
                                           UTILITY
descrizione
################################################################################################# 
*/
void get_req () {
    alarm(TIMEOUT);

    if(map[p.r][p.c].source_pid == 0) {
        alarm(0);
        sleep(1);
        raise(SIGALRM);
    }
    
    /* P(map[p.r][p.c].req_access_sem, 0); */
        if(read(map[p.r][p.c].req_pipe[R], &dest, sizeof(Pos)) < 0 && errno != EINTR) TEST_ERROR;

    /* V(map[p.r][p.c].req_access_sem, 0); */

    fprintf(logp, "%d (%d,%d) ==> (%d,%d)\n", getpid(), p.r+1, p.c+1 , dest.r+1, dest.c+1);
}

void write_log(FILE * logp) {
    fprintf(logp, "created %d @ (%d,%d): (PPID=%d | PGID=%d)\n",
            getpid(), p.r+1, p.c+1,
            getppid(), getpgid(getpid()));
}

void report() {

}

void pretend_doing (int sec) {
    sigset_t m;
    sigfillset(&m);
    sigdelset(&m, SIGQUIT);
    sigdelset(&m, SIGINT);
    sigdelset(&m, SIGALRM);
    /* sigdelset(&m, SIGSTOP); */

    sigprocmask(SIG_BLOCK, &m, NULL);

        sleep(sec);

    sigprocmask(SIG_UNBLOCK, &m, NULL);
}

/* 
################################################################################################# 
                                           MOVEMENT
descrizione
################################################################################################# 
*/

Dir get_direction(int r, int c){
    int dR, dC;
    Dir direction;

    if ((r - p.r) == 0 && (c - p.c) == 0) return NO;

    dR = (r - p.r) > 0;
    dC = (c - p.c) > 0;

    /* e' piu' urgente andare in verticale o in orizzontale ? */
    if(ABS(c-p.c) > ABS(r-p.r)) direction = dC * RIGHT + !dC * LEFT;
    else direction = dR * DOWN + !dR * UP;


    switch (direction) {
        case UP:
            if(map[p.r - 1][p.c].is_hole) return dC * RIGHT + !dC * LEFT;
            break;

        case DOWN:
            if(map[p.r + 1][p.c].is_hole) return dC * RIGHT + !dC * LEFT;
            break;


        case LEFT:
            if(map[p.r][p.c - 1].is_hole) return dR * DOWN + !dR * UP;
            break;
        
        case RIGHT:
            if(map[p.r][p.c + 1].is_hole) return dR * DOWN + !dR * UP;
            break;
    }

    return direction;
}


/* 
################################################################################################# 
                                           SIGNALS
descrizione
################################################################################################# 
*/
void set_signals () {
    struct sigaction sa;

    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGSTOP);
    sa.sa_flags = 0;
    sa.sa_mask = mask;
    sa.sa_handler = termination;
    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    sigemptyset(&mask);
    sa.sa_mask = mask;
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = resume;
    sigaction(SIGCONT, &sa, NULL);
}

/* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

void termination (int sig) {
    int i;

    alarm(0);

    V(map[p.r][p.c].cap_semid, 0);

    for(i = 0; i < SO_HEIGHT; i++) 
        if(shmdt(map[i])) TEST_ERROR;
    if(shmdt(map_row_ids)) TEST_ERROR;

    fprintf(logp, "unloaded %d\n\n", getpid());
    fclose(logp);

    if(operative) report();

    ALLSET(sync_all, 0, 0);

    switch(sig) {
        case SIGALRM: exit(TAXI_ABRT);

        case SIGINT:
        case SIGTERM: exit(EXIT_SUCCESS);
        
        default: exit(EXIT_FAILURE);
    }
}

 
void resume (int sig) {/* do nothing */}