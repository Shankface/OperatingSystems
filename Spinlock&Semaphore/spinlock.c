#include "spinlock.h"
#include "tas.h"

int spin_lock(char* spinlock){
    while(tas(spinlock) != 0);
    return *spinlock;
}

void spin_unlock(char* spinlock){
    *spinlock = 0;
}
