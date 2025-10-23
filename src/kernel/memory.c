#include "types.h"
#include "memory.h"
#include "paging.h"
#include "spinlock.h"

#define M_ALLOCATED 1
#define M_FREE      0

struct mem_header_struct {
    struct mem_header_struct *next;
    struct mem_header_struct *prev;
    size_t size;
    uint8 allocated;
};

extern struct spinlock_struct ref_cnt_lock;
static struct heap_header_struct kheap;
static struct spinlock_struct mem_lock;

static inline void increase_heap(struct heap_header_struct *heap)
{
    map(heap->heap_end, alloc_page(), PTE_R | PTE_W);    
    heap->heap_end++;
}

void *kalloc(size_t size)
{
    size += sizeof(struct mem_header_struct);   // increase size to make it include header
    struct mem_header_struct *n = 0;
    struct mem_header_struct *cur = kheap.head;
    while (cur && cur->size < size) {
        cur = cur->next;
    }

    if (cur == 0) {
        // There's no vacancy larger than requested size
        // increase kernel heap
    }
    n = (struct mem_header_struct *)((char *)cur + cur->size - size);

    n->allocated = M_ALLOCATED;
    n->size = size;
    n->prev = cur;
    n->next = (struct mem_header_struct *)((char *)n + size);

    return (void *)((char *)n + sizeof(struct mem_header_struct));
}

void kfree(void *mem)
{
    struct mem_header_struct *cur = (struct mem_header_struct *)((char *)mem - sizeof(struct mem_header_struct));
    size_t size = cur->size;
    struct mem_header_struct *next = cur->next;
    cur->allocated = M_FREE;
    if (next->allocated == M_FREE) {
        // merge
        cur->size = size + next->size;
        next->prev->next = cur;
        cur->prev = next->prev;
        cur->next = next->next;
    }
    else {
        // find next free block
        while (next && next->allocated == M_ALLOCATED) {
            next = next->next;
        }

        if (next) {
            cur->next = next;
            cur->prev = next->prev;
            cur->prev->next = cur;
        }
        else {
            // cur is end of free blocks
            cur->next = 0;
        }


    }

    kheap.free_size += size;

    return;
}

void memory_init()
{
    spinlock_init(&mem_lock, "mem");
    spinlock_init(&ref_cnt_lock, "ref_cnt");
    kheap.heap_end = alloc_page();     

    return;
}