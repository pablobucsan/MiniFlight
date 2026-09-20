

#ifndef SW_BUS_H
#define SW_BUS_H


#include "../common/subscribers.h"



typedef struct SWBus{
    SubscriptionTopic topic;
    Subscriber **sub_table;
}SWBus;

#endif //SW_BUS_H