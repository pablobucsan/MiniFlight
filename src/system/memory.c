
#include "../../include/system/memory.h"
#include "../../include/common/comps.h"
#include <stdlib.h>
#include <stdio.h>



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
 * Simple bump allocator for now
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

    //printf("[MEMORY SYSTEM] - Successful allocation of %zu bytes, old p: 0x%p, new p: 0x%p\n", size, result_p, (char *)result_p + size);


    mem_sys->memory_left -= size;
    mem_sys->next_free_p = (char *)mem_sys->next_free_p + size;
    
    return result_p;
}
