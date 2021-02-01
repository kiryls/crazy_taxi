#ifndef TAXI
#define TAXI

#define ABS(X) (((X)>0)*(X) + ((X)<=0)*(~(X)+1))

typedef enum {
    NO, 
    UP, 
    DOWN, 
    LEFT, 
    RIGHT
} Dir;

Pos          p;
Pos          dest;
int          TIMEOUT;
int          on_duty;
FILE*        logp;
Report       rep;
unsigned int timeTMP;

void init (const char * argv[]);
void get_req();
void write_log(FILE * logp);
void report();
void travel();
void move();
void set_signals();
void signal_handler(int sig);
void termination (int sig);
void resume (int sig);

void pretend_doing (int sec);

#endif 

