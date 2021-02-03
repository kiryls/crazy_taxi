/* 
################################################################################################# 
                                            TAXI

    Sono i processi piu' variabili all'interno della simulazione. Il loro unico scopo e' 
    quello di tentare di ottenere una richiesta da parte delle celle sorgenti e viaggiare
    fino ad arrivare alla destinazione. Si utilizza un meccanismo di prevenzione del 
    deadlock impostando degli allarmi ogni volta che un taxi fa un'attesa. 
################################################################################################# 
*/

#ifndef _TAXI
#define _TAXI

#define ABS(X) (((X)>0)*(X) + ((X)<=0)*(~(X)+1))

typedef enum { NO, UP, DOWN, LEFT, RIGHT } Dir;

Pos          p;
Pos          dest;
int          TIMEOUT;
int          on_duty;
Report       rep;


/* 
################################################################################################# 
                                              INIT 
descrizione
################################################################################################# 
*/
void init (const char * argv[]);

/* 
################################################################################################# 
                                           UTILITY
descrizione
################################################################################################# 
*/
void get_req();
void write_log();
void report();

/* 
################################################################################################# 
                                           MOVEMENT
descrizione
################################################################################################# 
*/
void travel();
void move();

/* 
################################################################################################# 
                                           SIGNALS
descrizione
################################################################################################# 
*/
void set_signals();
void signal_handler(int sig);

/* 
################################################################################################# 
                                          TERMINATION
descrizione
################################################################################################# 
*/
void termination (int sig);

#endif 

