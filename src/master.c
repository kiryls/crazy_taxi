#include "../headers/common.h"
#include "../headers/master.h"

int main(int argc, char *argv[]) {

    int i, j;
    char ** args;
    int status;
    int child;
    
    srand(time(NULL));

    if (load()) exit(EXIT_FAILURE);
    printf("loading completed\n");

    init_world();
    printf("world created\n");

    set_signals();
    printf("signals set\n");

    sync_simulation(sync_semaphore_id, 0, 0);

    printf("si puo' simulare\n");

    for(i = 0; i < config->SO_DURATION; i++) {
        sleep(1);
        raise(SIGALRM);
        print_map();
    }
    raise(SIGQUIT); /* wrap-up the simulation after SO_DURATION seconds */

    

    /* sleep(2); */
    while((child = wait(&status)) > 0) {
        printf("(%d) terminated\n", child);
    }

    if(errno == ECHILD) printf("all children terminated successfully\n");
    
    else {
        fprintf(stderr, "Error #%d: %s\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
    
    unload();

    printf("unloaded all structures\n");

    exit(EXIT_SUCCESS);


    /* switch(child = fork()) {
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
    } */
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

    /* put process in listening to SIGALRM */
    sigaction(SIGALRM,&sa,NULL);
    TEST_ERROR;

    /* set behaviour to wrap up */
    sa.sa_handler = wrap_up;

    /* put process in listening to SIGQUIT & SIGINT*/
    sigaction(SIGQUIT,&sa,NULL);
    TEST_ERROR;
    sigaction(SIGINT, &sa, NULL);
    TEST_ERROR;
}

void print_map_handler(int sig) {
    printf("raised periodic alarm\n");

    /* sigprocmask(SIG_BLOCK, &signal_mask, NULL);
    killpg(taxi_gpid, SIGSTOP);

    print_map();

    killpg(taxi_gpid, SIGCONT);
    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL); */
}

void wrap_up(int sig) {
    printf("raised wrap up signal\n");
    /* sigprocmask(SIG_BLOCK, &signal_mask, NULL);

    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL); */
}








