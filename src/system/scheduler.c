

#include "../../include/system/scheduler.h"
#include "../../include/common/comps.h"
#include "../../include/system/memory.h"
#include "../../include/components/health.h"
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

static SchedulerSystem *schdlr_sys = NULL;

void init_schdlr_sys()
{
    schdlr_sys = mem_sys_alloc(sizeof(SchedulerSystem));
    if (schdlr_sys == NULL){
        printf("[SCHEDULER SYSTEM] - Scheduler failed to allocate\n");
        exit(1);
    }

    schdlr_sys->components = mem_sys_alloc(sizeof(Component *) * MAX_COMPONENTS);
    if (schdlr_sys->components == NULL){
        printf("[SCHEDULER SYSTEM] - Scheduler components failed to allocate\n");
        exit(1);
    }

    schdlr_sys->ncomponents = 0;
    printf("[SCHEDULER SYSTEM] - ALIVE\n");
}   


void schdlr_sys_log_cmpnt(Component *component)
{
    if (schdlr_sys == NULL){
        printf("[SCHEDULER SYSTEM] - Cannot log a component onto a NULL scheduler\n");
        exit(1);
    }
    if (schdlr_sys->components == NULL){
        printf("[SCHEDULER SYSTEM] - Cannot log a component onto a NULL list of scheduler components\n");
        exit(1);
    }
    if (component == NULL){
        printf("[SCHEDULER SYSTEM] - Cannot log a component that is NULL\n");
        exit(1);
    }
    if (schdlr_sys->ncomponents + 1 >= MAX_COMPONENTS){
        printf("[SCHEDULER SYSTEM] - Cannot log a component, list is full\n");
        return;
    }

    schdlr_sys->components[schdlr_sys->ncomponents++] = component;

    printf("[SCHEDULER SYSTEM] - Logged a new component with type: %hu\n", component->cmpnt_type);
}


void schdlr_sys_tick_cmpnts()
{
    if (schdlr_sys == NULL){
        printf("[SCHEDULER SYSTEM] - Scheduler System is NULL. Cannot tick components\n");
        return;
    }

    if (schdlr_sys->components == NULL){
        printf("[SCHEDULER SYSTEM] - Scheduler components is NULL\n");
        return;
    }



    /**
     * Current scheduling strategy: Loop through the components in order
     */
    for (size_t i = 0; i < schdlr_sys->ncomponents; i++){
        printf("[SCHEDULER SYSTEM] - Looping...\n");
        Component *component = schdlr_sys->components[i];
        if (component == NULL){
            printf("[SCHEDULER SYSTEM] - Current component to tick is NULL\n");
            continue;
        }
        component->tick(component);
        sleep(1);
    }
}

