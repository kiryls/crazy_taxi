
Pos         p;
int         tot_reqs;
FILE        *logp;
sigset_t    sleep_mask;

void init(const char * argv[]);
void report();
void write_log(FILE * fp, Pos dest);
void set_signals();
void gen_req(int sig);
void termination(int sig);
void resume(int sig);
