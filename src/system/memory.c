
#include "../../include/system/memory.h"
#include "../../include/common/comps.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/**
 * @brief File-private single instance of the Memory System
 */
static MemorySystem mem_sys;


/**
 * @brief Initializes the Memory System.
 * Sets ```.total_memory``` to ```VEHICLE_MEMORY```
 * Sets ```.next_free_p``` to ```start_address```
 * Sets ```.memory_left``` to ```VEHICLE_MEMORY```
 * 
 * @param start_address The address of the first byte of the vehicle memory. Assumed to be ```NON-NULL```
 */
void init_mem_sys(void *start_address)
{

    mem_sys.total_memory = VEHICLE_MEMORY;
    mem_sys.next_free_p = (char *)start_address;
    mem_sys.memory_left = mem_sys.total_memory;

    printf("[MEMORY SYSTEM] - ALIVE\n");
}

/**
 * @brief Carve a block from the system memory arena for init-time, long-lived objects.
 * 
 * Use for:
 * 
 * - Structures and arrays (that may vary in size depending on the specific component)
 * that are created during start-up, remain allocated for the whole simulation, and are never returned to the arena
 * 
 * Do not use for:
 * 
 * - Per-```publish()```/per-```tick()``` traffic. Those use pre-created recyclable chunks.
 * 
 * @param size Size of the allocation block needed
 * 
 * @returns Pointer to a zeroed storage, or ```NULL``` if the arena does not have enough remaining bytes
 */
void *mem_sys_alloc(size_t size)
{  


    if (size > mem_sys.memory_left){
        printf("[MEMORY SYSTEM] - Not enough memory to satisfy the request\n");
        return NULL;
    }

    void *result_p = mem_sys.next_free_p;

    mem_sys.memory_left -= size;
    mem_sys.next_free_p = (char *)mem_sys.next_free_p + size;
    
    mem_sys_set(result_p, 0, size);

    return result_p;
}


/**
 * @brief Copies ```n``` bytes from ```src``` to ```dst```
 * 
 * @param dst Pointer to the destination stream to copy to
 * @param src Pointer to the source stream to copy from
 * @param n Number of bytes to copy
 */
void mem_sys_copy(void *dst, void *src, size_t n)
{
    memcpy(dst, src, n);
}   

/**
 * @brief Sets ```n``` bytes of a data stream starting at ```p``` to the value ```c```
 * 
 * @param p Pointer to the data stream
 * @param c Value to set
 * @param n Number of bytes
 */
void mem_sys_set(void *p, int c, size_t n)
{
    memset(p,c,n);
}

