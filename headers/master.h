#ifndef MASTER
#define MASTER

Config*     config;
char**      args;
pid_t       child_gpid;
pid_t       timer_id;
FILE*       fp;

int     load ();
void    unload ();
void    init_world();
void    gen_holes();
int     check_hole(int x, int y);
void    gen_timer();
void    gen_sources ();
void    gen_taxi ();
int     gen_one_taxi (int i, int j);
void    simulate ();
void    set_signals();
void    print_map_handler(int sig);
void    wrap_up(int sig);
void    print_map();
void    aftermath();
void    select_tops();
int     is_top(int r, int c);

#endif