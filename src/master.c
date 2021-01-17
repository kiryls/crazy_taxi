#include "../headers/common.h"

int main(int argc, char *argv[]) {

    int i, j;
    char ** args;
    int status;
    int aborted;
    int count;
    int child;

    srand(time(NULL));
    
    setup();

    map_id = shmget(IPC_PRIVATE, sizeof(Cell) * SO_WIDTH*SO_HEIGHT, S_IRUSR | S_IWUSR);
    TEST_ERROR;

    map = (Cell**) shmat(map_id, NULL, 0);
    TEST_ERROR;

    init_world(map);

    sync_simulation(sync_semaphore_id, 0, 0);

    alarm(1);

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








