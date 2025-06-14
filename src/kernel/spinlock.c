#include "spinlock.h"
#include "types.h"
#include "string.h"

// Initialize the lock
void spinlock_init(struct spinlock_struct *l, char *name)
{
    l->locked = 0;
    strcpy(l->name, name);
}

// Try to acquire the lock
void spinlock_acquire(struct spinlock_struct *l)
{
    int x;
    do {
        asm volatile("amoswap.w %0, %1, %2" : "=r" (x) : "r" (1), "m" (l->locked));
    }while (x == 1);
    return;
}

// Release the lock
void spinlock_release(struct spinlock_struct *l)
{
    l->locked = 0;
    return;
}

// Destroy the lock
void spinlock_destroy(struct spinlock_struct *l)
{
    return;
}