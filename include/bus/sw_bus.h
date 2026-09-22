

#ifndef SW_BUS_H
#define SW_BUS_H

#include <stddef.h>
// Ideally max components + max other systems that may use the bus
#define MAX_SUBSCRIBERS 20


typedef struct Subscriber Subscriber;



typedef struct SWBus{
    size_t nsubscribers;
    Subscriber *sub_table[MAX_SUBSCRIBERS];
}SWBus;


void init_swbus();
void swbus_publish();
void swbus_log_subscriber(Subscriber *s);

#endif //SW_BUS_H