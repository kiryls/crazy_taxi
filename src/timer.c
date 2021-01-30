#include "../headers/common.h"

int main(int argc, char const *argv[]) {
    int i;
    int DURATION;

    DURATION = atoi(argv[0]);
    pause_sem = atoi(argv[1]);

    sync_simulation(pause_sem, 0, 0);

    for(i = 0; i < DURATION; i++) {
        kill(getppid(), SIGALRM);
        sleep(1);
    }

    kill(getppid(), SIGTERM);

    exit(TIMER_EXIT);
}
