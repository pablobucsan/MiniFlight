


#ifndef COMPS_H
#define COMPS_H


typedef struct Subscriber Subscriber;
typedef struct Component Component;


typedef enum ComponentID{
    CMPNT_ID_NONE = 0x0,
    CMPNT_ID_HEALTH = 0x1,
    CMPNT_ID_INSTRUMENTS = 0x2,
    CMPNT_ID_GNC = 0x3,
}ComponentID;


/**
 * @brief Represents a component/module of the system
 */
typedef struct Component{
    /** Uniquely identifies the component */
    ComponentID cmpnt_id;  
    /** Pointer to subscriber state */
    Subscriber *subscriber;
    /** Periodic execution handler */
    void (*tick)(Component *cmpnt);
    /** Pointer to publisher state */
    void *publisher;
}Component;


#endif //COMPS_H