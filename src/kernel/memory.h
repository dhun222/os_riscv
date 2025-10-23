#pragma once
#include "types.h"
#include "paging.h"

typedef uint64 paddr_t;
typedef int64 vaddr_t;

struct heap_header_struct {
    vpn_t heap_start;
    vpn_t heap_end;
    struct mem_header_struct *head;
    size_t free_size;
};

void memory_init();