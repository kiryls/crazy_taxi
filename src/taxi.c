#include "../headers/common.h"
#include "../headers/taxi.h"




int main(int argc, char const *argv[]) {
    
    sigset_t alarm_mask;

    set_signals();
    
    init(argv);

    Z(sync_all);

    while(1) travel();

    exit(EXIT_FAILURE);
}




void init (const char * argv[]) {
    int i;

    map_id = atoi(argv[0]);
    p.r = atoi(argv[1]);
    p.c = atoi(argv[2]);
    TIMEOUT = atoi(argv[3]);
    sync_all = atoi(argv[4]);
    ledger_id = atoi(argv[5]);

    rep.taxi_id = getpid();
    rep.tot_length = 0;
    rep.time = 0;
    rep.completed_rides = 0;

    map_row_ids = (int*) shmat(map_id, NULL, 0);

    for(i = 0; i < SO_HEIGHT; i++) 
        map[i] = shmat(map_row_ids[i], NULL, 0);

    logfile = fopen("./logs/taxi.log", "a");

    ledger = shmat(ledger_id, NULL, 0);

    write_log();
}




void write_log() {
    fprintf(logfile, "created %d @ (%d,%d): (PPID=%d | PGID=%d)\n",
            getpid(), p.r+1, p.c+1,
            getppid(), getpgid(getpid()));
}

void report() {
    if(on_duty) fprintf(logfile, "%d **on duty** (%d,%d) --> (%d,%d): Tot cells=%d | Tot time=%.3f | Tot rides=%d\n", 
                            rep.taxi_id, p.r+1, p.c+1, dest.r+1, dest.c+1, rep.tot_length, 
                            rep.time, rep.completed_rides);

    else fprintf(logfile, "%d **off duty** (%d,%d): Tot cells=%d | Tot time=%.3f | Tot rides=%d\n", 
                            rep.taxi_id, p.r+1, p.c+1, rep.tot_length, 
                            rep.time, rep.completed_rides);

    if(rep.completed_rides == 0 && on_duty == 0) return; 

    P(ledger->taxi_section);

        ledger->completed_rides += rep.completed_rides;
        ledger->failed_rides += on_duty;
        
        if(rep.tot_length > ledger->best_distance.tot_length) {

            ledger->best_distance.taxi_id           = rep.taxi_id;
            ledger->best_distance.tot_length        = rep.tot_length;
            ledger->best_distance.time              = rep.time;
            ledger->best_distance.completed_rides   = rep.completed_rides;
        }

        if(rep.time > ledger->best_longevity.time) {

            ledger->best_longevity.taxi_id          = rep.taxi_id;
            ledger->best_longevity.tot_length       = rep.tot_length;
            ledger->best_longevity.time             = rep.time;
            ledger->best_longevity.completed_rides  = rep.completed_rides;

        } 

        if(rep.completed_rides > ledger->most_rides.completed_rides) {

            ledger->most_rides.taxi_id          = rep.taxi_id;
            ledger->most_rides.tot_length       = rep.tot_length;
            ledger->most_rides.time             = rep.time;
            ledger->most_rides.completed_rides  = rep.completed_rides;
        }

    V(ledger->taxi_section);
}




void get_req () {
    if(read(map[p.r][p.c].req_pipe[R], &dest, sizeof(Pos)) < 0 && errno != EINTR) TEST_ERROR;

    fprintf(logfile, "%d (%d,%d) ==> (%d,%d)\n", getpid(), p.r+1, p.c+1 , dest.r+1, dest.c+1);
}




Dir get_direction(){
    int dR, dC;
    Dir direction;

    dR = (dest.r - p.r);
    dC = (dest.c - p.c);

    if (dR == 0 && dC == 0) return NO;

    /* e' piu' urgente andare in verticale o in orizzontale ? */
    if(ABS(dC) > ABS(dR)) direction = (dC>0) * RIGHT + (dC<=0) * LEFT;
    else direction = (dR>0) * DOWN + (dR<=0) * UP;

    switch (direction) {
        case UP:
            if(map[p.r - 1][p.c].is_hole) direction = (dC>0) * RIGHT + (dC<=0) * LEFT;
            break;

        case DOWN:
            if(map[p.r + 1][p.c].is_hole) direction = (dC>0) * RIGHT + (dC<=0) * LEFT;
            break;

        case LEFT:
            if(map[p.r][p.c - 1].is_hole) direction = (dR>0) * DOWN + (dR<=0) * UP;
            break;
        
        case RIGHT:
            if(map[p.r][p.c + 1].is_hole) direction = (dR>0) * DOWN + (dR<=0) * UP;
            break;
    }

    switch (direction) {
        case UP:
            if(p.r-1 < 0) return DOWN;
            break;

        case DOWN:
            if(p.r+1 > SO_HEIGHT-1) return UP;
            break;

        case LEFT:
            if(p.c-1 < 0) return RIGHT;
            break;
        
        case RIGHT:
            if(p.c + 1 > SO_WIDTH-1) return LEFT;
            break;
    }
    return direction;
}




void travel () {
    Dir dir;
    float ride_time;
    struct timespec time;

    on_duty = 0;

    time.tv_nsec = map[p.r][p.c].travel_time;
    time.tv_sec  = 0;
    nanosleep(&time, (struct timespec *) NULL);

    alarm(TIMEOUT); 

    if(map[p.r][p.c].source_pid > 0) { get_req(); alarm(0); on_duty = 1; } 
    else raise(SIGALRM); 

    ride_time = 0;

    while((dir = get_direction()) != NO) {
        move(dir);

        rep.tot_length++;
        ride_time += (float) map[p.r][p.c].travel_time / 1000000000;
    }

    if(ride_time > rep.time) rep.time = ride_time;
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
    t.tv_sec  = 0;
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




void set_signals () {
    struct sigaction sa;
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGSTOP);
    sigaddset(&mask, SIGTERM);
    sa.sa_flags = 0;
    sa.sa_mask = mask;
    sa.sa_handler = termination;
    sigaction(SIGALRM, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
}




void termination (int sig) {
    int i;

    alarm(0);

    V(map[p.r][p.c].cap_semid);

    for(i = 0; i < SO_HEIGHT; i++) 
        if(shmdt(map[i])) TEST_ERROR;
    if(shmdt(map_row_ids)) TEST_ERROR;

    report();
    if(shmdt(ledger)) TEST_ERROR;

    fprintf(logfile, "unloaded %d\n\n", getpid());
    fclose(logfile);

    Z(sync_all);

    switch(sig) {
        case SIGALRM: exit(TAXI_ABORT);

        case SIGINT:
        case SIGTERM: exit(TAXI_EXIT);
        
        default: exit(EXIT_FAILURE);
    }
}
