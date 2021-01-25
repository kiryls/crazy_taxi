#include "../headers/common.h"
#include "../headers/source.h"

int main(int argc, char const *argv[]) {

    int i;
    int sem_ret;

    srand(getpid());
    
    init(argv);

    printf("child created w/ pid: %d, parent pid: %d, pos (%d,%d)\n", getpid(), getppid(), p.y+1, p.x+1);

    set_signals();
    
    /* chiudo la pipe richieste in lettura */
    
    sync_simulation(sync_semaphore_id, 0, -1);
    printf("source %d pronta\n", getpid());

    sleep(6);

    close(map[p.y][p.x].req_pipe[W]);
    
    for(i = 0; i < SO_HEIGHT; i++) shmdt(map[i]);
    shmdt(map);
    /* TEST_ERROR; */

    exit(EXIT_SUCCESS);
}

void init (const char * argv[]) {
    int i;

    map_id = atoi(argv[0]); 
    p.y = atoi(argv[1]);
    p.x = atoi(argv[2]);
    sync_semaphore_id = atoi(argv[3]);

    map_row_ids = (int*) shmat(map_id, NULL, 0);
    for(i = 0; i < SO_HEIGHT; i++) map[i] = shmat(map_row_ids[i], NULL, 0);

    map[p.y][p.x].source_pid = getpid();

    map[p.y][p.x].req_access_sem = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    semctl(map[p.y][p.x].req_access_sem, 0, SETVAL, 1);

    pipe(map[p.y][p.x].req_pipe);
    close(map[p.y][p.x].req_pipe[R]);
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
 