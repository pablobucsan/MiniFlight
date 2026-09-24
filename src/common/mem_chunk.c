
#include "../../include/common/mem_chunk.h"
#include "../../include/system/memory.h"
#include <stdint.h>
#include <stdio.h>

/**
 * Create a memory chunk of capacity 'capacity' for components to hold onto and recycle
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
void mem_chunk_clear(MemoryChunk *chunk)
{
    chunk->cursor = 0;
}

void mem_chunk_reserve(MemoryChunk *chunk)
{
    chunk->taken = 1;
}

void mem_chunk_unreserve(MemoryChunk *chunk)
{
    chunk->taken = 0;
    chunk->cursor = 0;
}

int mem_chunk_isreserved(MemoryChunk *chunk)
{
    return chunk->taken;
}

size_t mem_chunk_write(MemoryChunk *chunk, uint8_t *stream, size_t intended_bytes_to_write)
{
    if (chunk->data == NULL){
        printf("chunk darta null\n");
    }
    if (chunk == NULL || chunk->data == NULL || chunk->cursor >= chunk->capacity){
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

size_t mem_chunk_length(MemoryChunk *chunk)
{
    if (chunk == NULL){
        return 0;
    }

    return chunk->cursor;
}

size_t mem_chunk_read(MemoryChunk *chunk, uint8_t *out_stream, size_t intended_bytes_to_read)
{
    if (chunk == NULL){
        return 0;
    }

    size_t actual_bytes_to_read = intended_bytes_to_read;
    printf("We intend to read %zu bytes, payload length = %zu bytes, chunk cap = %zu\n",
    intended_bytes_to_read, chunk->cursor, chunk->capacity);
    /** We intend to read more bytes than the length of the chunk payload!!! */
    if (intended_bytes_to_read > chunk->cursor){
        actual_bytes_to_read = chunk->cursor;
    }

    mem_sys_copy(out_stream, chunk->data, actual_bytes_to_read);
    return actual_bytes_to_read;
}