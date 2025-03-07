#include "spinlock.h"
#include "types.h"
#include "string.h"

// Initialize the lock
void initSpinLock(struct spinLock_struct *l, char *name)
{
    asm volatile("");
    strcpy(l->name, name);
}

// Try to acquire the lock
void acquireSpinLock(struct spinLock_struct *l)
{
    int x;
    do {
        asm volatile("amoswap.w %0, %1, (%2)" : "=r" (x) : "r" (1), "r" (l->locked));
    }while (x == 1);
    return;
}

// Release the lock
void releaseSpinLock(struct spinLock_struct *l)
{
    l->locked = 0;
    return;
}

// Destroy the lock
void destSpinLock(struct spinLock_struct *l)
{
    return;
}