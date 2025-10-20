#pragma once
#include "types.h"
#include "riscv.h"
#include "paging.h"

typedef int64 pid_t;

struct task_struct {
    struct context_struct context;
    pid_t pid;
    satp_t satp;
};

// Linked list
struct node_struct {
    struct node_struct *next;
    struct task_struct proc;
};