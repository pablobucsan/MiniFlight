
#ifndef SUBSCRIBERS_H
#define SUBSCRIBERS_H

#include <stddef.h>

#define MAX_SUBSCRIPTIONS 5

typedef struct Component Component;



typedef enum MessageID{
    MSG_ID_NONE = 0x0,
    MSG_ID_HEALTH_1 = 0x1,
    MSG_ID_HEALTH_2 = 0x2,
    MSG_ID_HEALTH_3 = 0x3
}MessageID;



typedef struct Subscriber{
    size_t nmsg_id;
    MessageID sub_ids[MAX_SUBSCRIPTIONS];
    Component *component;
}Subscriber;

Subscriber *make_subscriber(Component *cmpnt);
void subscriber_sub_to_msg_id(Subscriber *s, MessageID msg_id);
void subscriber_unsub_from_msg_id(Subscriber *s, MessageID msg_id);

#endif //SUBSCRIBERS_H
