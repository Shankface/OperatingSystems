#ifndef SPINLOCK_H
#define SPINLOCK_H
#define N_PROC 64

int spin_lock(char* spinlock);

void spin_unlock(char* spinlock);

#endif