
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stddef.h>
#include <stdint.h>


#define MAX_COMPONENTS 10

/**
 * DESCRIPTION: Drive the temporal behavior of the simulation
 * 
 * ROLES:
 * Maintain a simple cyclic executive or priority-based dispatch loop
 * Call each component's periodic 'tick()' function at a defined rate
 * Support both periodic tasks and aperiodic / event-driven work
 * Provide a global simulation time source
 * Allow the system to be stepped (for demos and testing)
 */



typedef struct Component Component;


/**
 * @brief The structure responsible for ```tick()```-ing the components
 */
 typedef struct SchedulerSystem{
   /** Total number of components in the table */
    size_t ncomponents;
   /** Table of components logged to the Scheduler*/
    Component *components[MAX_COMPONENTS];
    /** Tracks the current tick frame */
    uint32_t current_tick_frame;
 }SchedulerSystem;


void init_schdlr_sys();
void schdlr_sys_log_cmpnt(Component *component);
void schdlr_sys_tick_cmpnts();
uint32_t schdlr_sys_current_frame();

#endif //SCHEDULER_H

