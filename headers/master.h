/* 
################################################################################################# 
                                            MASTER

    Il master e' il coordinatore di tutta la simulazione. Si serve di un processo timer che
    gli segnala periodicamente di stampare la mappa. Prima della simulazione prepara il terreno
    di gioco, dalle strutture dati ai processi coinvolti. Tutti si allineano sul semaforo SYNC
    in attesa che il master lo azzeri per la partenza. Finita la simulazione si fanno un po' 
    calcoli per le statistiche e infine si deallocano tutte le strutture utilizzate. 
################################################################################################# 
*/

#ifndef _MASTER
#define _MASTER

Config*     config;
FILE*       settings;
char**      args;
pid_t       child_gpid;
pid_t       timer_id;

/* 
################################################################################################# 
                                          LOAD & UNLOAD

    Vengono caricati/distrutti: 
        1. i parametri della simulazione dal file di configurazione
        2. il vettore che serve per passare gli argomenti ai processi
        3. il semaforo di SYNC della simulazione
        4. la struttura Ledger in cui riportare tutti i record
        5. il set delle celle Top (piu' trafficate)
################################################################################################# 
*/
int     load ();
void    unload ();

/* 
################################################################################################# 
                                              INITS

    L'init inizializza le strutture dati e la mappa della simulazione. Gen_holes dispone
    gli edifici.
################################################################################################# 
*/
void    init_world();
void    gen_holes();

/* 
################################################################################################# 
                                        PROCESS FACILITY

    Questi metodi servono a creare tutti i processi coinvolti nella simulazione.
################################################################################################# 
*/
void    gen_timer();
void    gen_sources ();
void    gen_taxi ();
int     gen_one_taxi (int i, int j);
void    respawn ();

/* 
################################################################################################# 
                                          SIMULATION

    La simulazione vera e propria. Consiste in un ciclo in cui il master attende (wait) la
    terminazione (ABORT) dei taxi per crearne altri. A fine ciclo, attende la terminazione
    (EXIT) di tutti i processi in gioco.
################################################################################################# 
*/
void    simulate ();

/* 
################################################################################################# 
                                            SIGNALS

    Set_signals specifica il comportamento da intraprendere per il master alla ricezione di
    alcuni segnali, in particolare: 
        - print_map_handler quando si riceve un SIGALRM
        - wrap_up quando si riceve un SIGTERM
################################################################################################# 
*/
void    set_signals();
void    print_map_handler(int sig);
void    wrap_up(int sig);

/* 
################################################################################################# 
                                            UTILITY

    Altre funzionalita' utili alla simulazione e al suo corretto svolgimento.
################################################################################################# 
*/
int     check_hole(int x, int y);
void    select_tops();
int     is_top(int r, int c);
void    print_map();
void    aftermath();

#endif