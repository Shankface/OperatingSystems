#ifndef SEM_H
#define SEM_H
#include "spinlock.h"
#include <stdlib.h>
#define N_PROC 64

struct sem{
    int count;
    char spinlock;
    int sleeping[N_PROC];
    int sleep_count[N_PROC]; 
    int woken_count[N_PROC]; 
    int signal_count[N_PROC];
};

void sem_init(struct sem* s, int count);

int sem_try(struct sem* s);

void sem_wait(struct sem* s, int my_procnum);

void sem_inc(struct sem* s);

#endif
