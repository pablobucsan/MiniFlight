


#ifndef COMPS_H
#define COMPS_H

typedef struct SchedulerSystem SchedulerSystem;
typedef struct HealthManager HealthManager;
typedef struct InstrumentsManager InstrumentsManager;

typedef enum ComponentID{
    CMPNT_ID_NONE = 0x0,
    CMPNT_ID_HEALTH = 0x1,
    CMPNT_ID_INSTRUMENTS_MNG = 0x2,
}ComponentID;


typedef struct Component{
    ComponentID cmpnt_id;
    void (*init)(SchedulerSystem *schdlr_sys);
    void (*tick)(Component *cmpnt);
    void (*shutdown)();
    union{
        HealthManager *health_mng;
        InstrumentsManager *instruments_mng;
    };
}Component;


#endif //COMPS_H