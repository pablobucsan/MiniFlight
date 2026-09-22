

#include "../../include/bus/sw_bus.h"
#include "../../include/system/memory.h"
#include "../../include/common/subscribers.h"
#include "../../include/common/comps.h"
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

    swbus->nsubscribers = 0;

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




