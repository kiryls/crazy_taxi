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

#define DENSE 0
#define STYLE 1

#if DENSE
    #define SO_WIDTH
    #define SO_HEIGHT
    #define CONFIG "./config/config_dense.txt"

#else 
    #define SO_HEIGHT 4 
    #define SO_WIDTH  6
    #define CONFIG "./config/config_custom.txt"
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

#define TAXI_ABORTED 123
#define R 0 /* READ form pipe */
#define W 1 /* WRITE to pipe */
#define NARGS 8
#define ABS(X) (((X)>0)*(X) + ((X)<=0)*(~(X)+1))

typedef enum {UP, DOWN, LEFT, RIGHT} Dir;

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
    int req_access_sem;
    int req_pipe[2];
    int cap_semid;
    int cell_cap;
    int travel_time;
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
    int STUD;
    /* 
        ### source_section_semid ###
        tot_reqs


        ### taxi_section_semid ###
        successful rides
        aborted rides
        (unfilled rides)

        int ** top cells

        cell-num-record taxi pid
        travel-time-record taxi pid
        successful-rides taxi pid
     */
} Ledger;

/* global vars */
Config *    config;

int         map_id;
int         *map_row_ids;
Cell        *map[SO_HEIGHT];

char        **args;

int         sync_sources_sem;
int         sync_taxi_sem;

sigset_t    mask;

pid_t       taxi_gpid;
pid_t       source_gpid;


/* implemented methods */
int     load ();
void    init_world();
void    gen_holes();
int     check_hole(int x, int y);
void    gen_sources ();
void    sync_simulation (int semid, int nsem, int value);
void    P (int semaphore, int index);
void    V (int semaphore, int index);
void    print_map();
void    unload ();

/* to implement methods */
void    gen_taxi ();
int     gen_one_taxi (int i, int j);
/* void set_signals (); */

/* void simulate (); */


#endif 

