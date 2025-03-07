#include "types.h"

struct spinLock_struct {
    // 0 : This lock is not held by any process, 1 : This lock is held by some process
    uint32 locked;
    // for debugging
    char name[10];
};

void initSpinLock(struct spinLock_struct *l, char *name);
void acquireSpinLock(struct spinLock_struct *l);
void releaseSpinLock(struct spinLock_struct *l);
void destSpinLock(struct spinLock_struct *l);