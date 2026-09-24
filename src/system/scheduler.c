

#include "../../include/system/scheduler.h"
#include "../../include/common/comps.h"
#include "../../include/system/memory.h"
#include "../../include/components/health.h"
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

SchedulerSystem schdlr_sys;

void init_schdlr_sys()
{

    schdlr_sys.ncomponents = 0;
    printf("[SCHEDULER SYSTEM] - ALIVE\n");
}   


void schdlr_sys_log_cmpnt(Component *component)
{

    if (component == NULL){
        printf("[SCHEDULER SYSTEM] - Cannot log a component that is NULL\n");
        exit(1);
    }
    if (schdlr_sys.ncomponents + 1 >= MAX_COMPONENTS){
        printf("[SCHEDULER SYSTEM] - Cannot log a component, list is full\n");
        return;
    }

    schdlr_sys.components[schdlr_sys.ncomponents++] = component;

    printf("[SCHEDULER SYSTEM] - Logged a new component with ID: %hu\n", component->cmpnt_id);
}


void schdlr_sys_tick_cmpnts()
{
    /**
     * Current scheduling strategy: Loop through the components in order
     */
    for (size_t i = 0; i < schdlr_sys.ncomponents; i++){
        Component *component = schdlr_sys.components[i];
        if (component == NULL){
            printf("[SCHEDULER SYSTEM] - Current component to tick is NULL\n");
            continue;
        }
        if (component->tick == NULL){
            printf("[SCHEDULER SYSTEM] - Current component to tick has a NUll tick function\n");
            continue;
        }
        component->tick(component);
        sleep(1);
    }
}

