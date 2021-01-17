#include "../headers/common.h"

int main(int argc, char const *argv[]) {
    
    int m_id;
    int index;
    Cell * cells;
    int i;

    m_id = atoi(argv[0]);
    index = atoi(argv[1]);

    cells = (Cell*) shmat(m_id, NULL, 0);
    TEST_ERROR;

    cells[index].is_edificio = index;
    cells[index].is_sorgente = 1 - index;
    cells[index].num_passaggi = index + 3;
    cells[index].tempo_percorrenza = 420 / (index+1);
    cells[index].pid = getpid();

    shmdt(cells);
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
    
}
 