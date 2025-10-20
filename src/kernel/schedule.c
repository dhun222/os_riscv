#include "types.h"
#include "schedule.h"
#include "spinlock.h"
#include "paging.h"

#define MAX_PROC 100

// Linked list
struct spinlock_struct pcb_lock;
struct node_struct *head, *tail;
uint16 n_proc;
pid_t pid_last;
struct task_struct task_pool[MAX_PROC];


void sched()
{

}

pid_t alloc_pid()
{
    pid_t ret = pid_last;
    pid_last++;
    return ret;
}

// Make new pcb node, allocate pid, stack
// Returns the address of new pcb
// Must acquire 'pcb_lock' first
struct task_struct *alloc_proc()
{
    struct task_struct *n = &task_pool[n_proc];
    n_proc++;    
    n->pid = alloc_pid();
    return n;
}

// Free pcb of terminated proc. 
// Return value for error check
int free_proc()
{
    return 0;
}

void sched_init()
{
    head = tail = 0;
    n_proc = 0;
    pid_last = 0;
    spinlock_init(&pcb_lock, "pcb");
}

// 
void proc_init()
{
    spinlock_acquire(&pcb_lock);
    struct task_struct *n = alloc_proc();
    // insert new proc

    // install pagetable
    asm volatile("csrr %0, satp" : "=r" (n->satp) : );

    // install user stack
    // load text, rodata, data

    
}
