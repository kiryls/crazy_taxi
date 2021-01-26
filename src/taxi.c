#include "../headers/common.h"
#include "../headers/taxi.h"

int main(int argc, char const *argv[]) {
    
    int i, j;

    init(argv);

    set_signals();

    sync_simulation(sync_taxi_sem, 0, -1);

    pause();

}

void init (const char * argv[]) {
    int i;

    map_id = atoi(argv[0]);
    p.r = atoi(argv[1]);
    p.c = atoi(argv[2]);

    rep.tot_length = 0;
    rep.tot_time = 0;
    rep.completed_rides = 0;

    map_row_ids = (int*) shmat(map_id, NULL, 0);
    for(i = 0; i < SO_HEIGHT; i++) map[i] = shmat(map_row_ids[i], NULL, 0);

    logp = fopen("/home/kiryls/Documents/Coding/project/logs/taxi.log", "a");
}

void set_signals () {
    struct sigaction sa;

    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGINT);

    sa.sa_flags = SA_RESTART;
    sa.sa_mask = mask;
    sa.sa_handler = signal_handler;

}

void signal_handler (int sig) {
    sigprocmask(SIG_BLOCK, &mask, NULL);

    switch (sig) {
    case SIGALRM:
        break;

    case SIGQUIT:
        break;

    case SIGINT:
        break;
    
    default:
        break;
    }

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}


 
void resume (int sig) {
    fprintf(logp, "[resuming taxi %d]\n", getpid());
}