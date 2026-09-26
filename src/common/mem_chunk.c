
#include "../../include/common/mem_chunk.h"
#include "../../include/system/memory.h"
#include <stdint.h>
#include <stdio.h>


/**
 * @brief Populates a memory chunk with initial values, sets the capacity and links it to the data it acts as a viewer to
 * 
 * @param raw_data A ```NON-NULL``` pointer to the raw data the chunk views
 * @param capacity The total capacity of the chunk data
 * 
 * @returns A MemoryChunk object initialized
 * 
 */
MemoryChunk mem_chunk_init(uint8_t *raw_data, size_t capacity)
{
    MemoryChunk mem_chunk;

    mem_chunk.capacity = capacity;
    mem_chunk.cursor = 0;
    mem_chunk.data = raw_data;
    mem_chunk.taken = 0;

    return mem_chunk; 
}

/**
 * Prepares a chunk to be written into, resetting payload length to 0
 */

/**
 * @brief Clears a memory chunk, effectively setting its cursor (length) to 0
 * 
 * @param chunk A ```NON-NULL``` pointer to the chunk
 */
void mem_chunk_clear(MemoryChunk *chunk)
{
    chunk->cursor = 0;
}


/**
 * @brief Reserves a memory chunk so it cannot be handed-off to multiple components
 * 
 * @param chunk A ```NON-NULL``` pointer to the chunk
 */
void mem_chunk_reserve(MemoryChunk *chunk)
{
    chunk->taken = 1;
}

/**
 * @brief Unreserves a memory chunk so it can be handed-off to another component
 * 
 * @param chunk A ```NON-NULL``` pointer to the chunk
 */
void mem_chunk_unreserve(MemoryChunk *chunk)
{
    chunk->taken = 0;
}

/**
 * @brief Checks whether a memory chunk is reserved
 * 
 * @param chunk A ```NON-NULL``` pointer to the chunk
 * 
 * @return 1 if the chunk is taken/reserved, 0 otherwise 
 */
int mem_chunk_isreserved(MemoryChunk *chunk)
{
    return chunk->taken;
}

/**
 * @brief Writes into the memory chunk data 
 * 
 * @param chunk A ```NON-NULL``` pointer to the chunk
 * @param stream The raw data stream we want to write into the chunk data
 * @param intended_bytes_to_write Number of bytes we intend to write
 * 
 * @returns Actual number of bytes written to the chunk data, capping it at its remaining space
 */
size_t mem_chunk_write(MemoryChunk *chunk, uint8_t *stream, size_t intended_bytes_to_write)
{
    if (chunk->cursor >= chunk->capacity){
        return 0;
    }

    size_t actual_bytes_to_write = intended_bytes_to_write;
    size_t remaining_space = chunk->capacity - chunk->cursor;

    if (actual_bytes_to_write > remaining_space){
        actual_bytes_to_write = remaining_space;
    }

    if (actual_bytes_to_write > 0){
        void *dst = &chunk->data[chunk->cursor];
        mem_sys_copy(dst, stream, actual_bytes_to_write);
        chunk->cursor += actual_bytes_to_write;
    }

    return actual_bytes_to_write;
}

/**
 * @brief Returns the length of the chunk data
 * 
 * @param chunk A ```NON-NULL``` pointer to the chunk
 */
size_t mem_chunk_length(MemoryChunk *chunk)
{
    return chunk->cursor;
}


/**
 * 
 * @brief Reads the chunk data and writes its content into ```out_stream```
 * 
 * @param chunk A ```NON-NULL``` pointer to the memory chunk to read
 * @param out_stream A ```NON-NULL``` pointer to the stream in which the chunk data will be written to
 * @param intended_bytes_to_read Number of bytes we intent to read from the chunk data
 * 
 * @returns The actual bytes read from the chunk data, capping it at its length
 */
size_t mem_chunk_read(MemoryChunk *chunk, uint8_t *out_stream, size_t intended_bytes_to_read)
{
    size_t actual_bytes_to_read = intended_bytes_to_read;
    size_t chunk_data_lenght = mem_chunk_length(chunk);
    // printf("We intend to read %zu bytes, payload length = %zu bytes, chunk cap = %zu\n",
    // intended_bytes_to_read, chunk_data_lenght, chunk->capacity);

    
    /** We intend to read more bytes than the length of the chunk data!!! */
    if (intended_bytes_to_read > chunk_data_lenght){
        actual_bytes_to_read = chunk_data_lenght;
    }

    mem_sys_copy(out_stream, chunk->data, actual_bytes_to_read);
    return actual_bytes_to_read;
}