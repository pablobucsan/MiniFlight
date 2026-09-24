

#include "../../include/bus/sw_bus.h"
#include "../../include/system/memory.h"
#include "../../include/common/subscribers.h"
#include "../../include/common/comps.h"
#include "../../include/common/message.h"
#include "../../include/common/mem_chunk.h"
#include <stdlib.h>
#include <stdio.h>

SWBus swbus = {0};

/**
 * 
 * DESCRIPTION: 
 * Initializes the Software Bus 
 * Sets default number of subscribers values
 * Initializes the memory chunks the Software Bus is supposed to hand off for components to form their messages
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
 * 
 * DESCRIPTION: 
 * Logs a subscriber onto the Software Bus subscribers table
 * 
 * PARAMETERS:
 * S: The subscriber to be added to the table
 */
void swbus_log_subscriber(Subscriber *s)
{

    if (s == NULL){
        printf("[SOFTWARE BUS] - Cannot log a NULL subscriber\n");
        exit(1);
    }

    if (swbus.nsubscribers + 1 >= MAX_SUBSCRIBERS){
        printf("[SOFTWARE BUS] - Cannot log a subcriber to software bus table, max subscribers reached\n");
        exit(1);
    }

    swbus.sub_table[swbus.nsubscribers++] = s;
}


/**
 * 
 * DESCRIPTION: 
 * Requests a memory chunk from the available ones within the Software Bus
 * 
 * PARAMETERS:
 * SIZE: The chunk size needed to fulfill the request
 * 
 * RETURNS:
 * A reference to one of the available memory chunks within the Software Bus
 */
MemoryChunk *swbus_rqst_mem_chunk(size_t size)
{

    if (size <= MSG_PACKET_1ST_CLASS_SIZE){
        for (int i = 0; i < MAX_1ST_CLASS_MSG_PACKETS_AT_ONCE; i++){
            MemoryChunk *mem_chunk = &swbus.mem_chunk_1st_class[i];
            if (mem_chunk_isreserved(mem_chunk) == 0){
                mem_chunk_reserve(mem_chunk);
                printf("[SWBUS] - Handed out chunk at slow 0x%p\n", mem_chunk);
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
 * DESCRIPTION: 
 * Publishes a message packet. 
 * 
 * PARAMETERS:
 * MSGPACKET: The message to be published
 * 
 */
void swbus_publish(MemoryChunk *msg_chunk, MessageID msg_id)
{
    printf("[SWBUS] - Publishing\n");
    for (size_t i = 0; i < swbus.nsubscribers; i++){
        Subscriber *s = swbus.sub_table[i];

        if (is_subbed_to_msg_id(s, msg_id)){
            int result = subscriber_enqueue_msg(s, msg_chunk);
            if (result == 0){
                printf("[SOFTWARE BUS] - Failed to enqueue a msg\n");
            }
        }
    }

    /** Now the slot can be reused to carry another message */
    mem_chunk_unreserve(msg_chunk);
}



