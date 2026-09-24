
#ifndef MEM_CHUNK_H
#define MEM_CHUNK_H

#include <stddef.h>
#include <stdint.h>


/**
 * 
 * @brief Acts as a viewer to the payload it points to via ```.data```
 */
typedef struct MemoryChunk
{
    /* Total allocated */
    size_t capacity;   
    /** Points to the last written byte, effectively
     the length of the data payload pointed at by ```.data``` 
    */    
    size_t cursor;   
    /*  Pointer to the raw data */        
    uint8_t *data;     
    /*  Whether the chunk is taken */        
    uint8_t taken;           
}MemoryChunk;

MemoryChunk mem_chunk_init(uint8_t *raw_data, size_t capacity);
size_t mem_chunk_write(MemoryChunk *chunk, uint8_t *stream, size_t intended_bytes_to_write);
size_t mem_chunk_length(MemoryChunk *chunk);
size_t mem_chunk_read(MemoryChunk *chunk, uint8_t *out_stream, size_t intended_bytes_to_read);
void mem_chunk_clear(MemoryChunk *chunk);
void mem_chunk_reserve(MemoryChunk *chunk);
void mem_chunk_unreserve(MemoryChunk *chunk);
int mem_chunk_isreserved(MemoryChunk *chunk);

#endif //MEM_CHUNK_H