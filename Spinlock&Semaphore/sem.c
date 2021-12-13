#include "sem.h"
#include "spinlock.h"
#include <signal.h> 
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void handler(int signum){
    return; 
}

void sem_init(struct sem* s, int count){
    s->count = count;
    s->spinlock = 0;
    int i;
    memset(s->sleeping, 0, N_PROC);
    memset(s->sleep_count, 0, N_PROC);
    memset(s->woken_count, 0, N_PROC);
    memset(s->signal_count, 0, N_PROC);
}

int sem_try(struct sem* s){ //return 1 if successfull, return 0 if count not positive
    int val = 0; 
    spin_lock(&s->spinlock); 
    if(s->count > 0){
        s->count--;
        val = 1;
    }
    spin_unlock(&s->spinlock);
    return val;
}


void sem_wait(struct sem *s, int my_procnum){
    while(1){

        spin_lock(&s->spinlock);
        if(s->count > 0){
            s->count--; //decrease count
            spin_unlock(&s->spinlock);
            return;
        }

        //block all signals
        sigset_t newmask,oldmask;
        sigfillset(&newmask); 
        signal(SIGUSR1, handler); 
        sigprocmask(SIG_BLOCK,&newmask,&oldmask);

        s->sleep_count[my_procnum]++;
        s->sleeping[my_procnum] = getpid(); //put pid into waitlist

        spin_unlock(&s->spinlock); 
        sigsuspend(&oldmask); //waits for signal

        //process woken up
        sigprocmask(SIG_SETMASK,&oldmask,NULL); //unblock everything
        signal(SIGUSR1,SIG_DFL);

        s->signal_count[my_procnum]++;
        s->sleeping[my_procnum] = 0; //take pid off waitlist

    }
    
}


void sem_inc(struct sem *s){
    spin_lock(&s->spinlock);
    s->count++; //increase count
    if(s->count > 0){ //if count is positive then wake any pids that were in waitlist
        int i;
        for(i = 0; i < N_PROC; i++){
            if(s->sleeping[i] != 0){
                kill(s->sleeping[i], SIGUSR1);
                s->sleeping[i] = 0;
                s->woken_count[i]++;
            }
        }
    }
    spin_unlock(&s->spinlock);
    return;    
}