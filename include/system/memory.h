
#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>

#define VEHICLE_MEMORY 65536 //64 KB




/**
 * DESCRIPTION: Controlled memory allocation and usage tracking for the simulation
 * 
 * RESPONBILITIES: 
 * Provide a simple deterministic allocator or memory pools for components that might need dynamic memory
 * Track total and per component memory usage
 * Publish memory statistics as telemetry
 * Detect and report allocation failures to the Health Manager
 */

typedef struct Component Component;
 
typedef struct MemorySystem{
    void *next_free_p;                /* CURRENT NEXT FREE ADDRESS*/
    size_t total_memory;             /* TOTAL_MEMORY  */
    size_t memory_left;              /* MEMORY LEFT*/
}MemorySystem;




void init_mem_sys(void *start_address);
void *mem_sys_alloc(size_t size);
void mem_sys_tick();


#endif //MEMORY_H