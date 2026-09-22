

#include "../../include/common/subscribers.h"
#include "../../include/common/comps.h"
#include "../../include/system/memory.h"

#include <stdlib.h>
#include <stdio.h>

Subscriber *make_subscriber(Component *cmpnt)
{
    if (cmpnt == NULL){
        printf("[SUBSCRIPTIONS] - Can't make a subscriber out of a NULL component\n");
        exit(1);
    }

    Subscriber *s = mem_sys_alloc(sizeof(Subscriber));
    if (s == NULL){
        printf("[SUBSCRIPTIONS] - Failed to allocate subscriber\n");
        exit(1);
    }

    s->component = cmpnt;
    s->nmsg_id = 0;

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

    s->sub_ids[s->nmsg_id++] = msg_id;
}

void subscriber_unsub_from_msg_id(Subscriber *s, MessageID msg_id)
{
    if (s == NULL){
        printf("[SUBSCRIPTIONS] - Cannot unsub from a msg id, subscriber is NULL\n");
        exit(1);
    }

    size_t index = 0;
    for (size_t i = 0; i < s->nmsg_id; i++){
        if (s->sub_ids[i] == msg_id){
            index = i;
            break;
        }
    }

    s->sub_ids[index] = MSG_ID_NONE;
    s->nmsg_id--;
}