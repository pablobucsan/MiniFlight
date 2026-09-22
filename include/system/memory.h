
#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdint.h>

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




typedef struct MemoryBuffer{
    size_t size;
    uint8_t taken;

    uint8_t buffer[];
}MemoryBuffer;



void init_mem_sys(void *start_address);
void *mem_sys_alloc(size_t size);
MemoryBuffer *mem_sys_create_buffer(size_t size);
void mem_sys_copy(void *dst, void *src, size_t n);
void mem_sys_set(void *p, int c, size_t n);


#endif //MEMORY_H