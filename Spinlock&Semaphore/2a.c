#include "spinlock.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>

struct NumSpin {
    int value;
    char spinlock;
};

int my_procnum = 0; 


int main(){
    struct NumSpin* number_with_spin = (struct NumSpin*)mmap(NULL, sizeof(struct NumSpin), PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
    struct NumSpin* number_without_spin = (struct NumSpin*)mmap(NULL, sizeof(struct NumSpin), PROT_READ|PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);

    number_with_spin->value = 0;
    number_with_spin->spinlock = 0;

    number_without_spin->value = 0;
    number_without_spin->spinlock = 0;

    int pid = getpid();
    int i;
    int children_pid[N_PROC];

    for(i = 0; i < N_PROC && pid != 0; i++){
        switch((pid = fork())){
            case -1: 
                perror("Error with fork");
                exit(errno);

            case 0:
                my_procnum = i+1;
                break;

            default:
                children_pid[i] = pid;
                break;
        }
    }

    for(i = 0; i < 100000; i++){
        //increment using spinlock
        spin_lock(&number_with_spin->spinlock);
        number_with_spin->value++;
        spin_unlock(&number_with_spin->spinlock);
        
        //increment without using spinlock
        number_without_spin->value++;
    }


    if(pid != 0){
        for(i = 0; i < N_PROC; i++){
            waitpid(children_pid[i], NULL, 0);
        }

        printf("%d children and 1 parent incrementing by 1 each 100000 times = %d\n", N_PROC, (N_PROC+1)*100000);
        printf("Value of number with spinlock: %d\nValue of number without spinlock: %d\n", number_with_spin->value, number_without_spin->value);
    }

    return 0; 
}