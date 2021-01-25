
Pos p;
int tot_reqs;
FILE * fp;

void init(const char * argv[]);
void write_log(FILE * fp, Pos dest);
void set_signals();
void termination(int sig);
void gen_req(int sig);
void resume(int sig);
