
#include "../../include/system/memory.h"
#include "../../include/common/comps.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



static MemorySystem *mem_sys = NULL;

void init_mem_sys(void *start_address)
{
    if (start_address == NULL){
        printf("[MEMORY SYSTEM] - Can't initialise Memory System as start address is NULL\n");
        exit(1);
    }

    mem_sys = start_address;
    mem_sys->total_memory = VEHICLE_MEMORY;
    mem_sys->next_free_p = ((char *)start_address + sizeof(MemorySystem));
    mem_sys->memory_left = mem_sys->total_memory - sizeof(MemorySystem);

    printf("[MEMORY SYSTEM] - ALIVE\n");
}

/**
 * Simple bump allocator for things that are always in memory. 
 * It zeroes the memory
 */
void *mem_sys_alloc(size_t size)
{  
    if (mem_sys == NULL){
        printf("[MEMORY SYSTEM] - Can't allocate since Memory System is NULL");
        exit(1);
    }

    if (size > mem_sys->memory_left ){
        printf("[MEMORY SYSTEM] - Not enough memory to satisfy the request\n");
        return NULL;
    }

    void *result_p = mem_sys->next_free_p;

    mem_sys->memory_left -= size;
    mem_sys->next_free_p = (char *)mem_sys->next_free_p + size;
    
    mem_sys_set(result_p, 0, size);

    return result_p;
}

/** Copy N bytes of a memory chunk onto another */
void mem_sys_copy(void *dst, void *src, size_t n)
{
    memcpy(dst, src, n);
}   

/** Fills the first N bytes of the memory area pointed by p with the constant c */
void mem_sys_set(void *p, int c, size_t n)
{
    memset(p,c,n);
}

