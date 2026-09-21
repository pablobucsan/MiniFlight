


#ifndef COMPS_H
#define COMPS_H

typedef struct SchedulerSystem SchedulerSystem;
typedef struct HealthManager HealthManager;


typedef enum ComponentType{
    CMPNT_HEALTH,
}ComponentType;


typedef struct Component{
    ComponentType cmpnt_type;
    void (*init)(SchedulerSystem *schdlr_sys);
    void (*tick)(Component *cmpnt);
    void (*shutdown)();
    union{
        HealthManager *health_mng;
    };
}Component;


#endif //COMPS_H