#include "types.h"
#include "spinlock.h"
#include "riscv.h"
#include "string.h"

void spinlock_init(struct spinlock_struct *l, char *name)
{
    l->locked = 0;
    l->hart = get_hartid();
    strcpy(l->name, name);

    return;
}

void spinlock_acquire(struct spinlock_struct *l)
{
    int x;
    do {
        asm volatile("amoswap.w %0, %1, %2" : "=r" (x) : "r" (1), "m" (l->locked));
    }while (x == 1);
    __sync_synchronize();
    l->hart = get_hartid();

    return;
}

void spinlock_release(struct spinlock_struct *l)
{
    l->hart = -1;
    __sync_synchronize();
    l->locked = 0;
    
    return;
}

