#include "sem.h"
#include <stdio.h>
#include <sys/mman.h>
#include <signal.h> 
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define SHELL_COUNT 3
#define TASK_COUNT 6

// void assignShell(int* fromShell, int* toShell){
//     *toShell = (*toShell + 1)%SHELL_COUNT;
//     if(*toShell == 0) *fromShell = (*fromShell + 1)%SHELL_COUNT;
//     if(*fromShell == *toShell) return assignShell(fromShell, toShell);
// }


int main(int argc, char* argv[]){
    
    int rockCount = 1;
    int moveCount = rockCount;

    if(argc > 2){
        rockCount = atoi(argv[1]);
        moveCount = atoi(argv[2]);
    }

    fprintf(stderr,"Shells: %d   Rocks: %d   Tasks: %d    Moves: %d\n", SHELL_COUNT, rockCount, TASK_COUNT, moveCount);
    
    struct sem* shells = (struct sem*)mmap(NULL, sizeof(struct sem)*SHELL_COUNT, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, 0, 0);

    int i, j, status;
    for(i = 0; i < SHELL_COUNT; i++){
        sem_init(shells+i,rockCount);
    }

    int pid = getpid();
    int my_procnum = 0;
    int children_pid[TASK_COUNT]; //array of task pids
    int fromShell[] = {0, 0, 1, 1, 2, 2};
    int toShell[] = {1, 2, 0, 2, 0, 1};

    for(i = 0; i < TASK_COUNT && pid != 0; i++){
        
        //assignShell(&fromShell, &toShell);

        switch((pid = fork())){
            case -1: 
                perror("Error with fork");
                exit(errno);

            case 0:
                my_procnum = i;
                fprintf(stderr, "VCPU %d starting, pid %d\n", my_procnum, getpid());
                break;

            default:
                children_pid[i] = pid;
                break;
        }
    }

    // In a child
    if(pid == 0){
        for(i = 0; i < moveCount; i++){
            sem_wait(shells + fromShell[my_procnum], my_procnum);
            sem_inc(shells + toShell[my_procnum]);          
        }
        fprintf(stderr, "VCPU %d done\n", my_procnum); 
    }

    // In parent
    if(pid != 0){
        fprintf(stderr, "Main process spawned all children, waiting...\n");
        for(i = 0; i < TASK_COUNT; i++){
            waitpid(children_pid[i], &status, 0);
            fprintf(stderr, "Child %d (pid %d) done and exited with %d, signal handler was invoked %d times\n", i, children_pid[i], WEXITSTATUS(status), shells[fromShell[i]].signal_count[i]);
        }

        for(i = 0; i < SHELL_COUNT; i++){
            fprintf(stderr, "\nShell #%d Count: %d\n", i, shells[i].count);
            for(j = 0; j < TASK_COUNT; j++){
                fprintf(stderr, "VCPU %d: Sleeps: %d Wakes: %d\n", j, shells[i].sleep_count[j], shells[i].woken_count[j]);
            }
            printf("\n");
        }
    }
    return 0; 
}
