
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stddef.h>

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

 typedef struct SchedulerSystem{
    size_t ncomponents;
    Component **components;
 }SchedulerSystem;


SchedulerSystem *init_schdlr_sys();
void schdlr_sys_log_cmpnt(SchedulerSystem *schdlr_sys, Component *component);
void schdlr_sys_tick_cmpnts(SchedulerSystem *schdlr_sys);

#endif //SCHEDULER_H

