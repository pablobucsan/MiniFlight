

#include "../../include/common/subscribers.h"
#include "../../include/common/comps.h"
#include "../../include/system/memory.h"
#include "../../include/common/message.h"
#include "../../include/components/health.h"
#include "../../include/common/mem_chunk.h"

#include <stdlib.h>
#include <stdio.h>



Subscriber *make_subscriber(size_t capacity, Queue_Policy policy)
{

    Subscriber *s = mem_sys_alloc(sizeof(Subscriber));
    if (s == NULL){
        printf("[SUBSCRIPTIONS] - Failed to allocate subscriber\n");
        exit(1);
    }

    /** Populate the subscriber */
    s->nmsg_id = 0;
    for (size_t i = 0; i < MAX_SUBSCRIPTIONS; i++){
        s->msg_ids[i] = MSG_ID_NONE;
    }

    s->msg_q.capacity = capacity;
    s->msg_q.head = 0;
    s->msg_q.tail = 0;
    s->msg_q.nmsgs = 0;
    s->msg_q.q_policy = policy;
    s->msg_q.msgs = mem_sys_alloc(sizeof(Msg_Packet) * capacity);

    if (s->msg_q.msgs == NULL){
        printf("[SUBSCRIPTIONS] - Failed to allocate for the message packets in the queue\n");
        exit(1);
    }

    return s;
}

void subscriber_sub_to_msg_id(Subscriber *s, MessageID msg_id)
{
    if (s == NULL){
        printf("[SUBSCRIPTIONS] - Cannot sub to a msg id, subscriber is NULL\n");
        exit(1);
    }

    if (s->nmsg_id + 1 >= MAX_SUBSCRIPTIONS){
        printf("[SUBSCRIPTIONS] - Cannot sub to a msg id, max subscriptions reached\n");
        return;
    }

    s->msg_ids[s->nmsg_id++] = msg_id;
}

void subscriber_unsub_from_msg_id(Subscriber *s, MessageID msg_id)
{
    if (s == NULL){
        printf("[SUBSCRIPTIONS] - Cannot unsub from a msg id, subscriber is NULL\n");
        exit(1);
    }

    size_t index = 0;
    for (size_t i = 0; i < s->nmsg_id; i++){
        if (s->msg_ids[i] == msg_id){
            index = i;
            break;
        }
    }

    s->msg_ids[index] = MSG_ID_NONE;
    s->nmsg_id--;
}


/**
 * 
 * 
 * @brief Processes the chunk containing the Message Packet delivered by the bus, copying it into 
 * subscriber's message queue
 * 
 * @param s The subscriber receiving the chunk
 * @param msg_chunk The chunk containing the Message Packet delivered by the bus
 * 
 * @returns 0 on error, 1 if successfull
 */
int subscriber_enqueue_msg(Subscriber *s, MemoryChunk *msg_chunk)
{
    /** Copy bus owned message packet into subscriber owned message packet queue */
    if (s == NULL){
        printf("[SUBSCRIPTIONS] - Cannot enqueue a msg packet, subscriber is NULL\n");
        exit(1);
    }

    /** Reference the subscribes queue, as we are going to modify it */
    Msg_Queue *msg_q = &s->msg_q;

    printf("[SUBSCRIPTIONS] - About to enqueue a msg in a q with %zu msgs\n", msg_q->nmsgs);

    /* Handle full queue scenarios */
    if (msg_q->nmsgs == msg_q->capacity){
        switch(msg_q->q_policy ){
            case Q_DROP_OLDEST:{
                /** Get the oldest index */
                msg_q->head = (msg_q->head + 1) % msg_q->capacity;
                msg_q->nmsgs--;
                break;
            }
            default:{
                return 0;
            }
        }
    }

    /** Copy the Message Packet from the chunk into the queue slot */
    Msg_Packet *msg_pkt = &msg_q->msgs[msg_q->tail];
    size_t chunk_length = mem_chunk_length(msg_chunk);
    size_t bytes_read = mem_chunk_read(msg_chunk, (uint8_t *)msg_pkt, chunk_length);
    
    if (bytes_read != chunk_length){
        printf("[SUBSCRIPTIONS] - Msg packet reading got truncated\n");
        exit(1);
    }

    msg_q->tail = (msg_q->tail + 1) % msg_q->capacity;
    msg_q->nmsgs++;

    printf("Finished enqueueing\n");
    return 1;
}

/**
 * @brief Dequeues a Message Packet from the subcriber's queue
 * 
 * @param s The subscriber from whose queue we will obtain the Message Packet
 * @param out_msg_packet A reference to copy into the dequeued Message Packet
 * 
 * @returns 0 on error or no msgs, 1 if successfull
 */
int subscriber_dequeue_msg(Subscriber *s, Msg_Packet *out_msg_packet)
{
    printf("Dequeueing a message\n");
    if (s == NULL){
        return 0;
    }

    Msg_Queue *msg_q = &s->msg_q;

    if (msg_q->nmsgs == 0){
        printf("No msgs\n");
        return 0;
    }


    *out_msg_packet = msg_q->msgs[msg_q->head];

    msg_q->head = (msg_q->head + 1) % msg_q->capacity;
    msg_q->nmsgs--;
    printf("Finished dequeueing a message\n");
    return 1;
}

int is_subbed_to_msg_id(Subscriber *s, MessageID msg_id)
{
    if (s == NULL){
        return 0;
    }

    for (size_t i = 0; i < s->nmsg_id; i++){
        if (s->msg_ids[i] == msg_id){
            return 1;
        }
    }

    return 0;
}
