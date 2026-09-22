

#include "../../include/bus/sw_bus.h"
#include "../../include/system/memory.h"
#include "../../include/common/subscribers.h"
#include "../../include/common/comps.h"
#include "../../include/common/message.h"
#include <stdlib.h>
#include <stdio.h>

SWBus *swbus = NULL;


void init_swbus()
{
    swbus = mem_sys_alloc(sizeof(SWBus));
    if (swbus == NULL){
        printf("[SOFTWARE BUS] - Software bus failed to allocate\n");
        exit(1);
    }

    /** Initial number of subscribers */
    swbus->nsubscribers = 0;
    /** Subscribers table is populated as they subify() */

    /** Create the memory buffer for the different sizes of packets */
    /** 1st class pool */
    for (int i = 0; i < MAX_1ST_CLASS_MSG_PACKETS_AT_ONCE; i++){
        swbus->mem_buffer_1st_class[i] = mem_sys_create_buffer(MSG_PACKET_1ST_CLASS_SIZE);
    }
    /** 2nd class pool */
    for (int i = 0; i < MAX_2ND_CLASS_PACKETS_AT_ONCE; i++){
        swbus->mem_buffer_2nd_class[i] = mem_sys_create_buffer(MSG_PACKET_2ND_CLASS_SIZE);
    }


    printf("[SOFTWARE BUS] - Alive\n");
}

void swbus_log_subscriber(Subscriber *s)
{
    if (swbus == NULL){
        printf("[SOFTWARE BUS] - Cannot log a subscriber to a NULL software bus\n");
        exit(1);
    }

    if (s == NULL){
        printf("[SOFTWARE BUS] - Cannot log a NULL subscriber\n");
        exit(1);
    }

    if (swbus->nsubscribers + 1 >= MAX_SUBSCRIBERS){
        printf("[SOFTWARE BUS] - Cannot log a subcriber to software bus table, max subscribers reached\n");
        exit(1);
    }

    swbus->sub_table[swbus->nsubscribers++] = s;
    
    printf("[SOFTWARE BUS] - Logged a new subscriber which has a component ID: %hu\n", s->component->cmpnt_id);

}

MemoryBuffer *swbus_rqst_mem_buffer(size_t size)
{
    /** Msg_Packet only weights the header as payload is Flexible Array Member */
    size_t total_size = sizeof(Msg_Packet) + size;

    if (total_size <= MSG_PACKET_1ST_CLASS_SIZE){
        for (int i = 0; i < MAX_1ST_CLASS_MSG_PACKETS_AT_ONCE; i++){
            MemoryBuffer *mem_buffer = swbus->mem_buffer_1st_class[i];
            if (mem_buffer->taken == 0){
                mem_buffer->taken = 1;
                return mem_buffer;
            }
        }

        return NULL;
    }

    if (total_size <= MSG_PACKET_2ND_CLASS_SIZE){
        for (int i = 0; i < MAX_2ND_CLASS_PACKETS_AT_ONCE; i++){
            MemoryBuffer *mem_buffer = swbus->mem_buffer_2nd_class[i];
            if (mem_buffer->taken == 0){
                mem_buffer->taken = 1;
                return mem_buffer;
            }
        }
        return NULL;
    }

    return NULL;
}


void swbus_publish(Msg_Packet *msg_packet)
{
    MessageID msg_id = msg_packet->msg_id;

    for (size_t i = 0; i < swbus->nsubscribers; i++){
        Subscriber *s = swbus->sub_table[i];
        for (size_t j = 0; j < s->nmsg_id; j++){
            if (s->sub_ids[j] == msg_id){
                Component *cmpnt = s->component;
                cmpnt->on_msg_received(cmpnt, msg_packet);
                break;
            }
        }
    }
}



