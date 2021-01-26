#include "../headers/common.h"
#include "../headers/source.h"

int main(int argc, char const *argv[]) {

    int i;

    srand(getpid());
    
    init(argv);

    set_signals();
    
    sync_simulation(sync_sources_sem, 0, -1);

    pause(); /* pause until SIGCONT form master */

    while(1) {
        sleep(2);
        raise(SIGALRM);
    }

    /* for(i = 0; i < 3; i++) raise(SIGALRM); */
}

void init (const char * argv[]) {
    int i;

    map_id = atoi(argv[0]); 
    p.r = atoi(argv[1]);
    p.c = atoi(argv[2]);
    sync_sources_sem = atoi(argv[3]);

    tot_reqs = 0;

    map_row_ids = (int*) shmat(map_id, NULL, 0);
    for(i = 0; i < SO_HEIGHT; i++) map[i] = shmat(map_row_ids[i], NULL, 0);

    map[p.r][p.c].source_pid = getpid();

    map[p.r][p.c].req_access_sem = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    semctl(map[p.r][p.c].req_access_sem, 0, SETVAL, 1);

    close(map[p.r][p.c].req_pipe[R]);

    logp = fopen("/home/kiryls/Documents/Coding/project/logs/src.log", "a");
}

void write_log(FILE * logp, Pos dest) {
    fprintf(logp, "%d @ (%d,%d): (PPID=%d | PGID=%d) request #%d -> (%d,%d)\n",
            getpid(), p.r+1, p.c+1,
            getppid(), getpgid(getpid()), 
            tot_reqs, dest.r+1, dest.c+1);
}

void set_signals () {
    struct sigaction sa;

    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGINT);

    sa.sa_flags = SA_RESTART;
    sa.sa_mask = mask;

    sa.sa_handler = termination;
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    
    sa.sa_handler = gen_req;
    sigaction(SIGALRM, &sa, NULL);

    sa.sa_handler = resume;
    sigaction(SIGCONT, &sa, NULL);
}

void gen_req (int sig) {
    int i;
    sigset_t alarm;
    Pos dest;

    sigemptyset(&alarm);
    sigaddset(&alarm, SIGALRM);

    /* sigprocmask(SIG_BLOCK, &alarm, NULL); */

    do{
        dest.r = rand() % SO_HEIGHT;
        dest.c = rand() % SO_WIDTH;
    }while(map[dest.r][dest.c].is_hole || ( dest.r == p.r && dest.c == p.c));

    P(map[p.r][p.c].req_access_sem, 0);
        write(map[p.r][p.c].req_pipe[W], &dest, sizeof(Pos));
    V(map[p.r][p.c].req_access_sem, 0);

    tot_reqs++;

    write_log(logp, dest);

    /* sigprocmask(SIG_UNBLOCK, &alarm, NULL); */
}

void termination (int sig) {
    int i;
    /* sigprocmask(SIG_BLOCK, &mask, NULL); */

    if(sig == SIGINT || sig == SIGQUIT) {

        close(map[p.r][p.c].req_pipe[W]);
    
        for(i = 0; i < SO_HEIGHT; i++) 
            if(shmdt(map[i])) TEST_ERROR;
        if(shmdt(map_row_ids)) TEST_ERROR;

        fclose(logp);

        fprintf(logp, "source (%d) terminated successfully\n", getpid());

        exit(EXIT_SUCCESS);

    }  else printf("source.c (#%3d): unhandled singnal\n", getpid() - getppid());

    /* sigprocmask(SIG_UNBLOCK, &mask, NULL); */
}

void resume (int sig) {
    fprintf(logp, "[resuming source %d]\n", getpid());
}
 