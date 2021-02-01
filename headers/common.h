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

#define DENSE 1
#define STYLE 1

#if DENSE
    #define SO_HEIGHT 10
    #define SO_WIDTH 20
    #define CONFIG "./config/config_dense.txt"

#else 
    #define SO_HEIGHT 20 
    #define SO_WIDTH  60
    #define CONFIG "./config/config_large.txt"
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
#define SOURCE_EXIT 4
#define TIMER_EXIT 5
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
    int is_hole; /* 1 se edificio, 0 altrimenti */
    int source_pid; /* pid source oppure 0 */
    int req_pipe[2]; /* descrittori read/write della pipe richieste */
    int cap_semid; /* semaforo della capienza */
    int cell_cap; 
    int travel_time;
    int update_traffic_sem; /* semaforo per aggiornare il traffico in una cella */
    int traffic;
} Cell;

typedef struct {
    int r;
    int c;
} Pos;

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
    pid_t           taxi_id;
    int             tot_length;
    /* struct timespec tot_time; */
    float           time;
    int             completed_rides; 
} Report;

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

typedef struct {
    int val;
    Pos p;
} Top;

/* global vars */

int         map_id;
int*        map_row_ids;
Cell*       map[SO_HEIGHT];
sigset_t    mask;
int         sync_all;
int         ledger_id;
Ledger*     ledger;
Top*        tops;

void    P (int semaphore);
void    V (int semaphore);
void    Z (int semaphore);

#endif 

