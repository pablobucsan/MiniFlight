


#ifndef COMPS_H
#define COMPS_H

typedef struct SchedulerSystem SchedulerSystem;
typedef struct HealthManager HealthManager;
typedef struct InstrumentsManager InstrumentsManager;
typedef struct Msg_Packet Msg_Packet;
typedef struct Subscriber Subscriber;

typedef enum ComponentID{
    CMPNT_ID_NONE = 0x0,
    CMPNT_ID_HEALTH = 0x5,
    CMPNT_ID_INSTRUMENTS = 0x2,
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
    void (*on_msg_received)(Component *cmpnt, Msg_Packet *msg_packet);
    /** Private manager context */
    void *mng;
}Component;


#endif //COMPS_H