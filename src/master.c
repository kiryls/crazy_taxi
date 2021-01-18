#include "../headers/common.h"

int main(int argc, char *argv[]) {

    int i, j;
    char ** args;
    int status;
    int aborted;
    int count;
    int child;
    
    
    srand(time(NULL));
    
    load();

    map_id = shmget(IPC_PRIVATE, sizeof(Cell) * SO_WIDTH*SO_HEIGHT, S_IRUSR | S_IWUSR);
    TEST_ERROR;

    map = (Cell**) shmat(map_id, NULL, 0);
    TEST_ERROR;

    init_world(map);

    sync_simulation(sync_semaphore_id, 0, 0);

    for(i = 0; i < config->SO_DURATION; i++) alarm(1);

    raise(SIGQUIT); /* wrap-up the simulation after SO_DURATION seconds */

    aborted = 0;

    while(wait(&status) > 0) {
        if(WEXITSTATUS(status) == TAXI_ABORTED) aborted++;
    }


    switch(child = fork()) {
        case -1:
            TEST_ERROR;
            exit(EXIT_FAILURE);
            break;

        case 0:
            args = malloc(2 * sizeof(char *));

            args[0] = malloc(sizeof(int));
            sprintf(args[0],"%d", map_id);

            args[1] = NULL;

            execvp("./reader", args);

            break;

        default:
            break;
    }


    shmctl(map_id, 0, IPC_RMID);
    TEST_ERROR;

    exit(EXIT_SUCCESS);
}

void set_signals() {
    struct sigaction sa;

    /* set signal mask */
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, SIGALRM); /* every second alarm */
    sigaddset(&signal_mask, SIGINT);  /* user interruption (CTRL-C) */
    sigaddset(&signal_mask, SIGQUIT); /* simulation wrap up */

    sa.sa_flags = SA_RESTART;
    sa.sa_mask = signal_mask;

    /* set behaviour to print map */
    sa.sa_handler = print_map_handler;

    /* put process in listening to SIGALRM & SIGINT */
    sigaction(SIGALRM,&sa,NULL);
    TEST_ERROR;
    sigaction(SIGINT, &sa, NULL);
    TEST_ERROR;

    /* set behaviour to wrap up */
    sa.sa_handler = wrap_up;

    /* put process in listening to SIGQUIT */
    sigaction(SIGQUIT,&sa,NULL);
    TEST_ERROR;
}

void print_map_handler(int sig) {
    sigprocmask(SIG_BLOCK, &signal_mask, NULL);

    

    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
}

void wrap_up(int sig) {
    sigprocmask(SIG_BLOCK, &signal_mask, NULL);

    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
}








