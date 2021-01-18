#include "../headers/common.h"

int main(int argc, char const *argv[]) {
    
    int map_id;
    int i, j;
    Cell * map;
    int i;

    map_id = atoi(argv[0]);

    map = (Cell*) shmat(map_id, NULL, 0);
    TEST_ERROR;


    shmdt(map);
    TEST_ERROR;

    return 0;
}

void set_signals () {
    struct sigaction sa;

    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGALRM);
    sigaddset(&signal_mask, SIGQUIT);
    sigaddset(&signal_mask, SIGINT);

    sigfillset(&all_signals);

    sa.sa_flags = SA_RESTART;
    sa.sa_mask = signal_mask;
    sa.sa_handler = signal_handler;

}

void signal_handler (int sig) {
    sigprocmask(SIG_BLOCK, &signal_mask, NULL);

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

    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
}
 