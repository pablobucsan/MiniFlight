

#include "../../include/common/subscribers.h"
#include "../../include/common/comps.h"
#include "../../include/system/memory.h"
#include "../../include/common/message.h"
#include "../../include/components/health.h"
#include "../../include/common/mem_chunk.h"

#include <stdlib.h>
#include <stdio.h>


/**
 * 
 * @brief Makes a subscriber, sets it fields as well as setting up its Msg_Q
 * 
 * @warning 
 * - On subscriber allocation failure, it proceeds to ```exit()```
 * - On message queue allocation failure, it proceeds to ```exit()```
 * 
 * @param capacity The total queue capacity
 * @param policy The queue policy which describes what actions to take upon receiving 
 * a message when the queue is full
 * 
 * @returns A pointer to the created subscriber
 * 
 */
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

    s->receive_channel = channel_init(CHANNEL_RECEPTION);

    return s;
}

/**
 * 
 * @brief Subscribes to a MessageID to receive messages under that key. 
 * 
 * @note
 * - If the subscriber has reached  ```MAX_SUBSCRIPTIONS```, the function does nothing
 * 
 * @param s A ```NON-NULL``` pointer to the subscriber to subscribe
 * @param msg_id The MessageID to add to the subscriber total subscriptions
 */
void subscriber_sub_to_msg_id(Subscriber *s, MessageID msg_id)
{
    if (s->nmsg_id >= MAX_SUBSCRIPTIONS){
        printf("[SUBSCRIPTIONS] - Cannot sub to a msg id, max subscriptions reached\n");
        return;
    }

    s->msg_ids[s->nmsg_id++] = msg_id;
}

/**
 * 
 * @brief Unsubscribes from a MessageID so the subscriber no longer receives messages under that key. 
 * 
 * @note
 * - If ```msg_id``` is not found within the subscriber's subscribed ids, 
 * the function does nothing.
 * 
 * - If the number of subscriptions is 0, the function does nothing
 * 
 * @param s A ```NON-NULL``` pointer to the subscriber
 * @param msg_id 
 */
void subscriber_unsub_from_msg_id(Subscriber *s, MessageID msg_id)
{

    if (s->nmsg_id == 0){
        return;
    }

    size_t index = 0;
    uint8_t found = 0;
    for (size_t i = 0; i < s->nmsg_id; i++){
        if (s->msg_ids[i] == msg_id){
            index = i;
            found = 1;
            break;
        }
    }

    if (!found){
        return;
    }

    s->msg_ids[index] = MSG_ID_NONE;
    s->nmsg_id--;
}


/**
 * 
 * 
 * @brief Processes the chunk containing the Message Packet delivered by the bus, copying it into a
 * subscriber's message queue slot.
 * 
 * @note 
 * - If the queue is full, the slot is decided based on Queue Policy
 * 
 * @param s A ```NON-NULL``` pointer to the subscriber receiving the chunk
 * @param msg_chunk A ```NON-NULL``` pointer to the chunk containing the Message Packet delivered by the bus
 *  * 
 */
void subscriber_enqueue_msg(Subscriber *s, MemoryChunk *msg_chunk)
{
    /** Copy bus owned message packet into subscriber owned message packet queue */

    /** Reference the subscribes queue, as we are going to modify it */
    Msg_Queue *msg_q = &s->msg_q;

    // printf("[SUBSCRIPTIONS] - About to enqueue a msg in a q with %zu msgs\n", msg_q->nmsgs);

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
                return;
            }
        }
    }

    /** Copy the Message Packet from the chunk into the queue slot */
    Msg_Packet *msg_pkt = &msg_q->msgs[msg_q->tail];
    size_t chunk_length = mem_chunk_length(msg_chunk);
    size_t bytes_read = mem_chunk_read(msg_chunk, (uint8_t *)msg_pkt, chunk_length);
    
    if (bytes_read != chunk_length){
        printf("[SUBSCRIPTIONS] - Msg packet copying into own slot got truncated\n");
        s->receive_channel.receive_lfs.receive_fault_count++;
        return;
    }

    msg_q->tail = (msg_q->tail + 1) % msg_q->capacity;
    msg_q->nmsgs++;

    // printf("Finished enqueueing\n");
}

/**
 * @brief Dequeues a Message Packet from the subcriber's queue
 * 
 * 
 * @param s A ```NON-NULL``` pointer to the subscriber from whose queue we will obtain the Message Packet
 * @param out_msg_packet A ```NON-NULL``` reference to copy into the dequeued Message Packet
 * 
 * @returns 0 on error or no msgs, 1 if successfull
 */
int subscriber_dequeue_msg(Subscriber *s, Msg_Packet *out_msg_packet)
{
    // printf("Dequeueing a message\n");

    Msg_Queue *msg_q = &s->msg_q;

    if (msg_q->nmsgs == 0){
        // printf("No msgs\n");
        return 0;
    }


    *out_msg_packet = msg_q->msgs[msg_q->head];

    msg_q->head = (msg_q->head + 1) % msg_q->capacity;
    msg_q->nmsgs--;
    // printf("Finished dequeueing a message\n");
    return 1;
}

/**
 * 
 * @brief Checks whether the subscriber is subscribed to the given MessageID
 * 
 * @param s A ```NON-NULL``` pointer to the subscriber
 * @param msg_id The MessageID we want to check whether it exists within the subscriber's subscriptions
 * 
 * @returns 1 if subscribed, 0 if not
 */
int is_subbed_to_msg_id(Subscriber *s, MessageID msg_id)
{
    for (size_t i = 0; i < s->nmsg_id; i++){
        if (s->msg_ids[i] == msg_id){
            return 1;
        }
    }

    return 0;
}
