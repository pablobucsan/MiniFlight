

#include "../../include/system/scheduler.h"
#include "../../include/common/comps.h"
#include "../../include/system/memory.h"
#include "../../include/components/health.h"
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>



/**
 * @brief File-private single instance of the Scheduler
 */
static SchedulerSystem schdlr_sys;


/**
 * @brief Initializes the Scheduler.
 * Sets its number of components to and current tick frame to 0
 */
void init_schdlr_sys()
{
    schdlr_sys.ncomponents = 0;
    schdlr_sys.current_tick_frame = 0;
    printf("[SCHEDULER SYSTEM] - ALIVE\n");
}   


/** 
 * @brief Logs a component to the Scheduler list
 * 
 * @param component A ```NON-NULL``` component to log to the table
 */
void schdlr_sys_log_cmpnt(Component *component)
{

    if (schdlr_sys.ncomponents + 1 >= MAX_COMPONENTS){
        printf("[SCHEDULER SYSTEM] - Cannot log a component, list is full\n");
        return;
    }

    if (component->tick == NULL){
        printf("[SCHEDULER SYSTEM] - Cannot add the component with ID = %i, as it has NULL tick() function\n", component->cmpnt_id);
        return;
    }

    schdlr_sys.components[schdlr_sys.ncomponents++] = component;

    printf("[SCHEDULER SYSTEM] - Logged a new component with ID: %i\n", component->cmpnt_id);
}


/**
 * 
 * @brief Loops the Scheduler table of components and ```tick()```s them in order.
 * Repeats forever
 */
void schdlr_sys_tick_cmpnts()
{
    /**
     * Current scheduling strategy: Loop through the components in order
     */
    while (1){
        printf("======================== START TICK :%u =================== \n", schdlr_sys.current_tick_frame);
        for (size_t i = 0; i < schdlr_sys.ncomponents; i++){
            Component *component = schdlr_sys.components[i];
            component->tick(component);
            sleep(1);
        }
        printf("======================== FINISH TICK :%u =================== \n",schdlr_sys.current_tick_frame);

        /** Update the frame */
        schdlr_sys.current_tick_frame++;

    }
}

/**
 * 
 * @brief Queries the Scheduler System for the current tick frame
 * 
 * @returns The current tick frame
 */
uint32_t schdlr_sys_current_frame()
{
    return schdlr_sys.current_tick_frame;
}

