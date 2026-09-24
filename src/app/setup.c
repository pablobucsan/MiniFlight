




#include "../../include/system/memory.h"
#include "../../include/system/scheduler.h"
#include "../../include/components/health.h"
#include "../../include/components/instruments.h"
#include "../../include/bus/sw_bus.h"
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

    /** CREATE THE MEMORY SYSTEM */
    init_mem_sys(start_p);

    /** CREATE THE SCHEDULER */
    init_schdlr_sys();

    /** CREATE THE SOFTWARE BUS */
    init_swbus();
    
    /** CREATE EVERY OTHER COMPONENT, SELF REGISTER WITH THE SCHEDULER AND LOG ITSELF TO THE SOFTWARE BUS SUBSCRIPTIONS TABLE*/
    init_instruments_cmpnt();
    init_health_cmpnt();
}

void run()
{
    initialise();
    printf("============= Init finished. About to tick components ========\n");
    schdlr_sys_tick_cmpnts();
}
