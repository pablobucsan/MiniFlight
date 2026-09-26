

#include "../../include/bus/sw_bus.h"
#include "../../include/system/memory.h"
#include "../../include/common/subscribers.h"
#include "../../include/common/comps.h"
#include "../../include/common/message.h"
#include "../../include/common/mem_chunk.h"
#include <stdlib.h>
#include <stdio.h>


/**
 * @brief File-private Software Bus singleton
 */
static SWBus swbus = {0};


/**
 * @brief Initializes the Software Bus
 * 
 * - Sets to 0 the number of subscribers
 * 
 * - Initializes every Memory Chunk the Software Bus owns, acting as a viewer to the respective raw data pool
 * 
 */
void init_swbus()
{

    /** Initial number of subscribers */
    swbus.nsubscribers = 0;
    /** Subscribers table is populated as they subify() */

    /** Create the memory buffer for the different sizes of packets */
    /** 1st class pool */
    for (int i = 0; i < MAX_1ST_CLASS_MSG_PACKETS_AT_ONCE; i++){
        swbus.mem_chunk_1st_class[i] = mem_chunk_init(
                                        swbus.raw_data_1st_class[i], 
                                        MSG_PACKET_1ST_CLASS_SIZE);
    }
    /** 2nd class pool */
    for (int i = 0; i < MAX_2ND_CLASS_PACKETS_AT_ONCE; i++){
        swbus.mem_chunk_2nd_class[i] = mem_chunk_init(
                                            swbus.raw_data_2nd_class[i], 
                                            MSG_PACKET_2ND_CLASS_SIZE);
    }

    printf("[SOFTWARE BUS] - Alive\n");
}

/**
 * @brief Logs a subscriber onto the Software Bus' subscriber table
 * 
 * @warning 
 * - On max subscribers reached, it proceeds to ```exit()```
 * 
 * @param s A ```NON-NULL``` pointer to the subscriber to add to the table
 */
void swbus_log_subscriber(Subscriber *s)
{

    if (swbus.nsubscribers >= MAX_SUBSCRIBERS){
        printf("[SOFTWARE BUS] - Cannot log a subcriber to software bus table, max subscribers reached\n");
        exit(1);
    }

    swbus.sub_table[swbus.nsubscribers++] = s;
}



 /**
  * 
  * @brief Requests a MemoryChunk from the available ones that the Software Bus owns
  * 
  * @note 
  * - Upon finding a suitable chunk, reserves it
  * 
  * @param size Size of the chunk needed to fulfill the request
  * 
  * @return A ```MAY-BE-NULL``` pointer to the Memory Chunk
  */
MemoryChunk *swbus_rqst_mem_chunk(size_t size)
{

    if (size <= MSG_PACKET_1ST_CLASS_SIZE){
        for (int i = 0; i < MAX_1ST_CLASS_MSG_PACKETS_AT_ONCE; i++){
            MemoryChunk *mem_chunk = &swbus.mem_chunk_1st_class[i];
            if (mem_chunk_isreserved(mem_chunk) == 0){
                mem_chunk_reserve(mem_chunk);
                // printf("[SWBUS] - Handed out chunk at slow 0x%p\n", mem_chunk);
                return mem_chunk;
            }
        }

        return NULL;
    }

    if (size <= MSG_PACKET_2ND_CLASS_SIZE){
        for (int i = 0; i < MAX_2ND_CLASS_PACKETS_AT_ONCE; i++){
            MemoryChunk *mem_chunk = &swbus.mem_chunk_2nd_class[i];
            if (mem_chunk_isreserved(mem_chunk) == 0){
                mem_chunk_reserve(mem_chunk);
                return mem_chunk;
            }
        }
        return NULL;
    }

    return NULL;
}

/**
 * 
 * @brief Releases a previously requested MemoryChunk. Effectively unreserves it and clears it
 * 
 * @param msg_chunk A ```NON-NULL``` pointer to a requested MemoryChunk
 */
void swbus_release_chunk(MemoryChunk *msg_chunk)
{
    mem_chunk_unreserve(msg_chunk);
    mem_chunk_clear(msg_chunk);
}

/**
 * @brief Routes the Memory Chunk containing the Message Packet to the subscribers subscribed to the MessageID
 * 
 * @note 
 * - For every subscriber interested in the MessageID, enqueues the message
 * - Upon completion of all deliveries, unreserves the given chunk and clears it to prepare for recycling
 * 
 * @param msg_chunk A ```NON-NULL``` pointer to the Memory Chunk containing the Message Packet
 * @param msg_id The MessageID of the Message Packet to route the chunk based on
 * 
 */
void swbus_publish(MemoryChunk *msg_chunk, MessageID msg_id)
{
    // printf("[SWBUS] - Publishing\n");
    for (size_t i = 0; i < swbus.nsubscribers; i++){
        Subscriber *s = swbus.sub_table[i];

        if (is_subbed_to_msg_id(s, msg_id)){
            subscriber_enqueue_msg(s, msg_chunk);
        }
    }

    /** Now the slot can be reused to carry another message */
    mem_chunk_unreserve(msg_chunk);
    mem_chunk_clear(msg_chunk);
}



