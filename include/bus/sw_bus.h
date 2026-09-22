

#ifndef SW_BUS_H
#define SW_BUS_H

#include <stddef.h>

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
typedef struct MemoryBuffer MemoryBuffer;
typedef struct Msg_Packet Msg_Packet;

typedef struct SWBus{
    size_t nsubscribers;
    Subscriber *sub_table[MAX_SUBSCRIBERS];

    MemoryBuffer *mem_buffer_1st_class[MAX_1ST_CLASS_MSG_PACKETS_AT_ONCE];
    MemoryBuffer *mem_buffer_2nd_class[MAX_2ND_CLASS_PACKETS_AT_ONCE];
}SWBus;


void init_swbus();
void swbus_publish(Msg_Packet *msg_packet);
void swbus_log_subscriber(Subscriber *s);
MemoryBuffer *swbus_rqst_mem_buffer(size_t size);

#endif //SW_BUS_H