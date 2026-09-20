




#include "../../include/system/memory.h"
#include "../../include/system/scheduler.h"
#include "../../include/components/health.h"
#include <stdlib.h>
#include <stdio.h>


SchedulerSystem *initialise()
{
    /** INIT MEMORY POOL FOR THE PROGRAM */
    void *start_p = malloc(VEHICLE_MEMORY);
    if (start_p == NULL){
        printf("Vehicle memory allocation failed\n");
        exit(1);
    }

    /** CREATE THE MEMORY SYSTEM MODULE */
    init_mem_sys(start_p);

    /** CREATE THE SCHEDULER */
    SchedulerSystem *schdlr_sys = init_schdlr_sys();

    /** CREATE EVERY OTHER COMPONENT AND SELF REGISTER WITH THE SCHEDULER */
    init_health_cmpnt(schdlr_sys);

    return schdlr_sys;
}

void run()
{
    SchedulerSystem *schdlr_sys = initialise();
    schdlr_sys_tick_cmpnts(schdlr_sys);
}
