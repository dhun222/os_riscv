#include "types.h"

struct spinlock_struct {
    // 0 : This lock is not held by any process, 1 : This lock is held by some process
    int locked;
    // for debugging
    char name[10];
    int hart;   // hart holding this lock
};

void spinlock_init(struct spinlock_struct *l, char *name);
void spinlock_acquire(struct spinlock_struct *l);
void spinlock_release(struct spinlock_struct *l);