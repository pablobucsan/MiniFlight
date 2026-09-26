
#ifndef SUBSCRIBERS_H
#define SUBSCRIBERS_H

#include <stddef.h>
#include "message.h"
#include "channel.h"

#define MAX_SUBSCRIPTIONS 5

typedef struct Component Component;
typedef struct MemoryChunk MemoryChunk;

typedef enum Queue_Policy
{
    Q_DROP_OLDEST,
}Queue_Policy;

typedef struct Msg_Queue
{
    /** Capacity of the queue */
    size_t capacity;
    size_t nmsgs;
    /** Index of oldest Message Packet */
    size_t head;                 
    /** Index to place next Message Packet */
    size_t tail;                 

    /** Pointer to a contiguous block of memory 
     * containting ```.capacity``` Message Packets. Which is where
     * the Software Bus will copy into the message is routing
    */
    Msg_Packet *msgs;     
    
    /** Policy according to which messages are dropped if
     * queue is full
     */
    Queue_Policy q_policy;
}Msg_Queue;


typedef struct Subscriber{
    /** How many subscriptions the specific subscriber has */
    size_t nmsg_id;
    /** Array of the MessageID subscriptions */
    MessageID msg_ids[MAX_SUBSCRIPTIONS];
    /** Contiguous block of memory that itself contains a 
     * pointer to all the Message Packet copies the Software Bus
     * has placed into the queue
     */
    Msg_Queue msg_q;
    /**  */
    Channel receive_channel;
}Subscriber;

Subscriber *make_subscriber(size_t capacity, Queue_Policy policy);
void subscriber_sub_to_msg_id(Subscriber *s, MessageID msg_id);
void subscriber_unsub_from_msg_id(Subscriber *s, MessageID msg_id);

void subscriber_enqueue_msg(Subscriber *s, MemoryChunk *msg_chunk);
int subscriber_dequeue_msg(Subscriber *s, Msg_Packet *out_msg_packet);

int is_subbed_to_msg_id(Subscriber *s, MessageID msg_id);
#endif //SUBSCRIBERS_H
