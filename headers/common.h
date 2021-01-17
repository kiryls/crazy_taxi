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

#if DENSE
#define SO_WIDTH
#define SO_HEIGHT
#define CONFIG "./config/config_dense.txt"

#else 
#define SO_HEIGHT 4 
#define SO_WIDTH  4
#define CONFIG "./config/config_custom.txt"

#endif

#define TAXI_ABORTED 123

#define TEST_ERROR    if (errno) {dprintf(STDERR_FILENO,		\
					  "%s:%d: PID=%5d: Error %d (%s)\n", \
					  __FILE__,			\
					  __LINE__,			\
					  getpid(),			\
					  errno,			\
					  strerror(errno)); }

typedef struct {
    int is_hole; /* 1 se edificio, 0 altrimenti */
    int source_pid; /* pid source oppure 0 */
    int req_semid;
    int req_shmid;
    int cap_semid;
    int travel_time;
    int num_passes;
} Cell;

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

/* global vars */
Config * config;
int map_id;
Cell ** map;
int sync_semaphore_id;
sigset_t signal_mask;
sigset_t all_signals;


/* implemented methods */
int setup ();
void init_world(Cell ** map);
void gen_buildings(Cell ** map);
int check_hole(int x, int y, Cell ** map);
void gen_sources ();
void sync_simulation (int semid, int nsem, int value);
void P (int semaphore, int index);
void V (int semaphore, int index);

/* to implement methods */
void gen_taxi ();
// void set_signals ();
void init_sync_semaphores ();
/* void simulate (); */
void cleanup ();

#endif 

