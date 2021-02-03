/* 
################################################################################################# 
                                            SOURCE

    Le sorgenti sono processi ancorati ad una cella in particolare. Vengono create in numero
    limitato all'inizio della simulazione e distrutte solo alla fine. Il loro unico scopo e'
    quello di generare periodicamente le richieste finche' non arriva il segnale SIGTERM di
    terminazione della simulazione.
################################################################################################# 
*/

#ifndef _SOURCE
#define _SOURCE

Pos         p;
int         tot_reqs;

/* 
################################################################################################# 
                                              INIT 
descrizione
################################################################################################# 
*/
void init(const char * argv[]);

/* 
################################################################################################# 
                                           UTILITY
descrizione
################################################################################################# 
*/
void report();
void write_log(Pos dest);

/* 
################################################################################################# 
                                           SIGNALS
descrizione
################################################################################################# 
*/
void set_signals();
void gen_req(int sig);

/* 
################################################################################################# 
                                          TERMINATION
descrizione
################################################################################################# 
*/
void termination(int sig);

#endif