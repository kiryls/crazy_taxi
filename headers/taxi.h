#ifndef TAXI
#define TAXI

typedef struct {
    /* taxi report on every ride */
    int tot_length;
    int tot_time;
    int completed_rides; 
} Report;

Pos p;
int tot_reqs;
FILE * logp;
Report rep;

void set_signals();
void signal_handler(int sig);
void resume (int sig);

#endif 
