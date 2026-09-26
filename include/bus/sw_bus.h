

#ifndef SW_BUS_H
#define SW_BUS_H

#include <stddef.h>

#include "../common/mem_chunk.h"
#include "../common/message.h"
// Ideally max components + max other systems that may use the bus
#define MAX_SUBSCRIBERS 20


/** How many messages of this size can be published and not yet consumed 
 * at a time
 */
#define MAX_1ST_CLASS_MSG_PACKETS_AT_ONCE 2 
#define MAX_2ND_CLASS_PACKETS_AT_ONCE 2

/** Sizes of the different message placket classes */
#define MSG_PACKET_1ST_CLASS_SIZE 128
#define MSG_PACKET_2ND_CLASS_SIZE 256


typedef struct Subscriber Subscriber;
typedef struct Msg_Packet Msg_Packet;


typedef struct SWBus{
    size_t nsubscribers;
    Subscriber *sub_table[MAX_SUBSCRIBERS];

    /** Memory slots for 1st class packets */
    MemoryChunk mem_chunk_1st_class[MAX_1ST_CLASS_MSG_PACKETS_AT_ONCE];
    uint8_t raw_data_1st_class[MAX_1ST_CLASS_MSG_PACKETS_AT_ONCE][MSG_PACKET_1ST_CLASS_SIZE];

    /** Memory slots for 2nd class packets */
    MemoryChunk mem_chunk_2nd_class[MAX_2ND_CLASS_PACKETS_AT_ONCE];
    uint8_t raw_data_2nd_class[MAX_2ND_CLASS_PACKETS_AT_ONCE][MSG_PACKET_2ND_CLASS_SIZE];
}SWBus;





void init_swbus();
void swbus_publish(MemoryChunk *msg_chunk, MessageID msg_id);
void swbus_log_subscriber(Subscriber *s);
MemoryChunk *swbus_rqst_mem_chunk(size_t size);
void swbus_release_chunk(MemoryChunk *msg_chunk);

#endif //SW_BUS_H