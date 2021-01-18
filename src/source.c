#include "../headers/common.h"

int main(int argc, char const *argv[]) {

    Cell * cells;
    Pos p;

    srand(getpid());

    map_id = atoi(argv[0]);
    p.y = atoi(argv[1]);
    p.x = atoi(argv[2]);

    cells = (Cell*) shmat(map_id, NULL, 0);
    TEST_ERROR;
    
    set_signals();

    sync_simulation(sync_semaphore_id, 0, -1);


    
    shmdt(cells);
    TEST_ERROR;

    return 0;
}

void set_signals () {
    struct sigaction sa;

    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGALRM);
    sigaddset(&signal_mask, SIGUSR1);
    sigaddset(&signal_mask, SIGINT);

    sigfillset(&all_signals);

    sa.sa_flags = SA_RESTART;
    sa.sa_mask = signal_mask;
    sa.sa_handler = signal_handler;

}

void signal_handler (int sig) {
    sigprocmask(SIG_BLOCK, &signal_mask, NULL);

    switch (sig) {
    case SIGALRM: /* source: genera req */
        break;

    case SIGQUIT: /* master: so_duration */
        break;

    case SIGINT: /* utente: ctrl + c */
        raise(SIGQUIT);
        break;
    
    default:
        printf("source.c (#%3d): unhandled singnal\n", getpid() - getppid());
        break;
    }

    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
}
 