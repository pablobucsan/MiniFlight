
#ifndef SUBSCRIBERS_H
#define SUBSCRIBERS_H

typedef struct Component Component;


typedef enum SubscriptionTopic{
    TOPIC_HEALTH,
    TOPIC_SENSOR,
}SubscriptionTopic;



typedef struct Subscriber{
    SubscriptionTopic sub_topic;
    Component *component;
}Subscriber;


#endif //SUBSCRIBERS_H
