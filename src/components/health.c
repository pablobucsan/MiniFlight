


#include "../../include/components/health.h"
#include "../../include/system/memory.h"
#include "../../include/system/scheduler.h"
#include "../../include/common/comps.h"
#include "../../include/common/subscribers.h"
#include "../../include/bus/sw_bus.h"
#include <stdio.h>
#include <stdlib.h>


/** Forward declaration of functions to reference them */
void init_health_cmpnt();
void health_cmpnt_tick(Component *health_cmpnt);

void health_cmpnt_subify(Component *health_cmpnt)
{
    /** Make the subscriber */
    Subscriber *health_sub = make_subscriber(health_cmpnt);
    /** Add its subscription to IDs */

    /** Log to the Software Bus table */
    swbus_log_subscriber(health_sub);
}

void init_health_cmpnt()
{
    Component *health_cmpnt = mem_sys_alloc(sizeof(Component));
    if (health_cmpnt == NULL){
        printf("[HEALTH COMPONENT] - Health component failed to allocate\n");
        exit(1);
    }

    HealthManager *health_mng = mem_sys_alloc(sizeof(HealthManager));
    if (health_mng == NULL){
        printf("[HEALTH COMPONENT] - Health manager as part of health component failed to allocate\n");
        exit(1);
    }

    /** Populate the component */
    health_cmpnt->cmpnt_id = CMPNT_ID_HEALTH;
    health_cmpnt->health_mng = health_mng;
    health_cmpnt->init = init_health_cmpnt;
    health_cmpnt->tick = health_cmpnt_tick;
    health_cmpnt->shutdown = NULL;

    /** Log it to the Scheduler */
    schdlr_sys_log_cmpnt(health_cmpnt);

    /** Log it to the Software Bus table */
    health_cmpnt_subify(health_cmpnt);

    printf("[HEALTH COMPONENT] - Alive\n");
}

void health_cmpnt_tick(Component *health_cmpnt)
{

    if (health_cmpnt == NULL){
        printf("[HEALTH COMPONENT] - Can't tick NULL component\n");
        return;
    }
    if (health_cmpnt->cmpnt_id != CMPNT_ID_HEALTH){
        printf("[HEALTH COMPONENT] - Wrong component has reached Health Component Tick function\n");
        exit(1);
    }
    printf("[HEALTH COMPONENT] - Health status: %hu\n", health_cmpnt->health_mng->state);
}