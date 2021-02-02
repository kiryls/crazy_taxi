#include "../headers/common.h"
#include "../headers/source.h"

/* 
################################################################################################# 
                                            SOURCE
descrizione
################################################################################################# 
*/
int main(int argc, char const *argv[]) {

    srand(getpid());
    
    init(argv);

    set_signals();
    
    Z(sync_all);

    while(1) {
        raise(SIGALRM);
        sleep(2);
    }

    exit(EXIT_FAILURE);
}




void init (const char * argv[]) {
    int i;

    map_id = atoi(argv[0]); 
    p.r = atoi(argv[1]);
    p.c = atoi(argv[2]);
    sync_all = atoi(argv[3]);
    ledger_id = atoi(argv[4]);

    tot_reqs = 0;

    map_row_ids = (int*) shmat(map_id, NULL, 0);

    for(i = 0; i < SO_HEIGHT; i++) 
        map[i] = shmat(map_row_ids[i], NULL, 0);

    ledger = shmat(ledger_id, NULL, 0);

    close(map[p.r][p.c].req_pipe[R]);

    logfile = fopen("/home/kiryls/Documents/Coding/project/logs/src.log", "a");
}




void report () {
    P(ledger->source_section);
        ledger->tot_requests += tot_reqs;
    V(ledger->source_section);
}




void write_log(Pos dest) {
    fprintf(logfile, "%d @ (%d,%d): (PPID=%d | PGID=%d) request #%d -> (%d,%d)\n",
            getpid(), p.r+1, p.c+1,
            getppid(), getpgid(getpid()), 
            tot_reqs, dest.r+1, dest.c+1);
}




void set_signals () {
    struct sigaction sa;
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGTERM);
    sa.sa_flags = SA_RESTART;
    sa.sa_mask = mask;
    sa.sa_handler = termination;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    sigemptyset(&mask);
    sigaddset(&mask, SIGALRM);
    sa.sa_mask = mask;
    sa.sa_handler = gen_req;
    sigaction(SIGALRM, &sa, NULL);
}




void gen_req (int sig) {
    int i;
    sigset_t alarm;
    Pos dest;

    do{
        dest.r = rand() % SO_HEIGHT;
        dest.c = rand() % SO_WIDTH;
    }while(map[dest.r][dest.c].is_hole || ( dest.r == p.r && dest.c == p.c));

    if(write(map[p.r][p.c].req_pipe[W], &dest, sizeof(Pos)) < 0 && errno != EINTR) TEST_ERROR;

    tot_reqs++;

    write_log(dest);
}




void termination (int sig) {
    int i;

    close(map[p.r][p.c].req_pipe[W]);

    for(i = 0; i < SO_HEIGHT; i++) 
        if(shmdt(map[i])) TEST_ERROR;
    if(shmdt(map_row_ids)) TEST_ERROR;

    report();
    if(shmdt(ledger)) TEST_ERROR;

    fprintf(logfile, "source (%d) terminated successfully\n\n", getpid());
    fclose(logfile);

    Z(sync_all);

    exit(SOURCE_EXIT);
}

 