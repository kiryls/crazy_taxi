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

    Z(sync_all);

    while(1) travel();
 
    /* get_req();

    raise(SIGALRM); */

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

    /* dest.r = dest.c = -1; */

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

void write_log(FILE * logp) {
    fprintf(logp, "created %d @ (%d,%d): (PPID=%d | PGID=%d)\n",
            getpid(), p.r+1, p.c+1,
            getppid(), getpgid(getpid()));
}

void report() {
    if(on_duty) fprintf(logp, "%d on duty (%d,%d) --> (%d,%d): Tot cells=%d | Tot time=%d | Tot rides=%d\n", 
                            getpid(), p.r+1, p.c+1, dest.r+1, dest.c+1, rep.tot_length, rep.tot_time, rep.completed_rides);

    else fprintf(logp, "%d on (%d,%d): Tot cells=%d | Tot time=%d | Tot rides=%d\n", 
                            getpid(), p.r+1, p.c+1, rep.tot_length, rep.tot_time, rep.completed_rides);
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

void get_req () {
    /* P(map[p.r][p.c].req_access_sem, 0); */
    if(read(map[p.r][p.c].req_pipe[R], &dest, sizeof(Pos)) < 0 && errno != EINTR) TEST_ERROR;

    /* V(map[p.r][p.c].req_access_sem, 0); */

    fprintf(logp, "%d (%d,%d) ==> (%d,%d)\n", getpid(), p.r+1, p.c+1 , dest.r+1, dest.c+1);
}

Dir get_direction(){
    int dR, dC;
    Dir direction;

    if ((dest.r - p.r) == 0 && (dest.c - p.c) == 0) return NO;

    dR = (dest.r - p.r) > 0;
    dC = (dest.c - p.c) > 0;

    /* e' piu' urgente andare in verticale o in orizzontale ? */
    if(ABS(dest.c-p.c) > ABS(dest.r-p.r)) direction = dC * RIGHT + !dC * LEFT;
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
                                           MOVEMENT
descrizione
################################################################################################# 
*/


void travel () {
    Dir dir;

    alarm(TIMEOUT); 

    on_duty = 0;
    /* dest.r = dest.c = -1; */ /* set no goal */
    if(map[p.r][p.c].source_pid > 0) { get_req(); alarm(0); on_duty = 1; } 
    else { alarm(0); sleep(1); raise(SIGALRM); }

    while((dir = get_direction()) != NO) {
        move(dir);

        rep.tot_length++;
        rep.tot_time += map[p.r][p.c].travel_time;
    }

    rep.completed_rides++;
}

void move (Dir dir) {
    struct timespec t;
    Pos next;

    switch(dir) {
        case UP:    next.r = p.r-1;     next.c = p.c;       break;
        case DOWN:  next.r = p.r+1;     next.c = p.c;       break;
        case LEFT:  next.r = p.r;       next.c = p.c-1;     break;
        case RIGHT: next.r = p.r;       next.c = p.c+1;     break;
    }

    t.tv_nsec = map[next.r][next.c].travel_time;
    t.tv_sec = 0;

    nanosleep(&t, (struct timespec *) NULL);
    
    alarm(TIMEOUT);

    P(map[next.r][next.c].cap_semid);
    alarm(0);
    
    V(map[p.r][p.c].cap_semid);

    p.r = next.r;
    p.c = next.c;

    P(map[p.r][p.c].update_traffic_sem);
        map[p.r][p.c].traffic++;
    V(map[p.r][p.c].update_traffic_sem);
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

    V(map[p.r][p.c].cap_semid);

    for(i = 0; i < SO_HEIGHT; i++) 
        if(shmdt(map[i])) TEST_ERROR;
    if(shmdt(map_row_ids)) TEST_ERROR;

    report();

    fprintf(logp, "unloaded %d\n\n", getpid());
    fclose(logp);

    Z(sync_all);

    switch(sig) {
        case SIGALRM: exit(TAXI_ABORT);

        case SIGINT:
        case SIGTERM: exit(EXIT_SUCCESS);
        
        default: exit(EXIT_FAILURE);
    }
}

 
void resume (int sig) {/* do nothing */}