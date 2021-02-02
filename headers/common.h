#ifndef COMMON
#define COMMON

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif /* _GNU_SOURCE */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/stat.h>

#define CUSTOM 1
#define DENSE 1
#define STYLE 1

#if CUSTOM
    #define SO_HEIGHT 6
    #define SO_WIDTH 8
    #define CONFIG "./config/config_custom.txt"
#else
#if DENSE
    #define SO_HEIGHT 10
    #define SO_WIDTH 20
    #define CONFIG "./config/config_dense.txt"

#else 
    #define SO_HEIGHT 20 
    #define SO_WIDTH  60
    #define CONFIG "./config/config_large.txt"
#endif

#endif

#if STYLE
    #define HOLE "\x1b[37m\x1b[47m" 
    #define SOURCE "\x1b[42m\x1b[30m"  
    #define ENDSTYLE "\x1b[0m" 
    #define BUSY "\x1b[31m"
#else
    #define HOLE "" 
    #define SOURCE ""  
    #define ENDSTYLE "" 
    #define BUSY ""
#endif

#define TAXI_ABORT 3
#define TAXI_EXIT 4
#define SOURCE_EXIT 5
#define TIMER_EXIT 6
#define R 0 /* READ form pipe */
#define W 1 /* WRITE to pipe */
#define NARGS 8

#define TEST_ERROR    if (errno) {dprintf(STDERR_FILENO, \
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno)); }



typedef struct {    
    int SO_TAXI; 
    int SO_SOURCES; 
    int SO_HOLES;   
    int SO_TOP_CELLS; 
    int SO_CAP_MIN; 
    int SO_CAP_MAX; 
    int SO_TIMENSEC_MIN; 
    int SO_TIMENSEC_MAX;
    int SO_TIMEOUT; 
    int SO_DURATION; 
} Config;

typedef struct {
    int r;
    int c;
} Pos;

/* 
################################################################################################# 
                                            CELLA

    Ogni cella della matrice e' una struttura dati che permette di memorizzare info sul
    traffico, sulle caratteristiche (sorgente, edificio o vuota). Contiene ID di semafori
    e pipes per sincronizzare e far dialogare i processi taxi e le sorgenti. 
################################################################################################# 
*/
typedef struct {
    int is_hole;
    int source_pid; 
    int req_pipe[2]; 
    int cap_semid;
    int cell_cap; 
    int travel_time;
    int update_traffic_sem; 
    int traffic;
} Cell;

/* 
################################################################################################# 
                                            REPORT
    
    Il report che ogni taxi compila durante il suo ciclo di vita. E' come una scatola nera
    contenente informazioni che serviranno per aggiornare le statistiche globali della 
    simulazione.
################################################################################################# 
*/
typedef struct {
    pid_t           taxi_id;
    int             tot_length;
    float           time;
    int             completed_rides; 
} Report;

/* 
################################################################################################# 
                                            LEDGER
     
    Il registro in cui vengono depositate le informazioni riguardanti la simulazione. Viene
    aggiornato in modo concorrenziale e mantenuto condiviso tra tutti i processi della
    simulazione (i.e. master, source & taxi).
################################################################################################# 
*/
typedef struct {
    int source_section;
    int tot_requests;

    int taxi_section;
    int completed_rides;
    int failed_rides;
    int unfilled_rides;

    Report best_distance;
    Report best_longevity;
    Report most_rides;
} Ledger;

/* 
################################################################################################# 
                                            TOP
     
    Le celle che vengono registrate in questa struttura sono un sottoinsieme di celle della
    matrice attraverso cui sono passati piu' taxi.
################################################################################################# 
*/
typedef struct {
    int val;
    Pos p;
} Top;

/* 
################################################################################################# 
                                            GLOBAL VARS
     
    Principalmente sono ID di strutture dati e semafori che verranno condivise tra tutti i
    processi.
################################################################################################# 
*/
int         map_id;
int*        map_row_ids;
Cell*       map[SO_HEIGHT];
sigset_t    mask;
int         sync_all;
int         ledger_id;
Ledger*     ledger;
Top*        tops;

/* 
################################################################################################# 
                                    HOLD & RELEASE SEMAPHORES

    Wrapper dei metodi per impegnare/rilasciare un certo semaforo.
################################################################################################# 
*/
void P (int semaphore);
void V (int semaphore);

/* 
################################################################################################# 
                                        ZERO SEMAPHORE

    Questa funzione ha l'unico scopo di sospendere un processo in attesa che il semaforo
    diventi nullo.
################################################################################################# 
*/
void Z (int semaphore);

#endif 

