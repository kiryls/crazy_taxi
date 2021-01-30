#include "../headers/common.h"

int main(int argc, char const *argv[]) {
    int i;
    int DURATION;

    DURATION = atoi(argv[0]);
    sync_all = atoi(argv[1]);

    ALLSET(sync_all, 0, 0);

    for(i = 0; i < DURATION; i++) {
        kill(getppid(), SIGALRM);
        sleep(1);
    }

    kill(getppid(), SIGTERM);

    exit(TIMER_EXIT);
}
