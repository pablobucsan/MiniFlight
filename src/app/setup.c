




#include "../../include/system/memory.h"
#include "../../include/system/scheduler.h"
#include "../../include/components/health.h"
#include <stdlib.h>
#include <stdio.h>


void initialise()
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
    init_schdlr_sys();

    /** CREATE EVERY OTHER COMPONENT AND SELF REGISTER WITH THE SCHEDULER */
    init_health_cmpnt();
}

void run()
{
    initialise();
    schdlr_sys_tick_cmpnts();
}
