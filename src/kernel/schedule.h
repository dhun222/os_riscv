#pragma once
#include "types.h"
#include "riscv.h"
#include "paging.h"
#include "memory.h"

typedef int16 pid_t;
enum state_enum {RUNNABLE, RUNNING, SLEEP};

struct task_struct {
    struct context_struct context;
    struct heap_header_struct user_heap;
    pagetable_t gpd;
    pid_t ppid;
    pid_t pid;
    enum state_enum state;
};

// Linked list
struct node_struct {
    struct node_struct *next;
    struct task_struct proc;
};

void sched();