


#include "../../include/components/health.h"
#include "../../include/system/memory.h"
#include "../../include/system/scheduler.h"
#include "../../include/common/comps.h"
#include <stdio.h>
#include <stdlib.h>


/** Forward declaration of functions to reference them */
void init_health_cmpnt();
void health_cmpnt_tick();

void init_health_cmpnt()
{
    Component *health_cmpnt = mem_sys_alloc(sizeof(Component));
    if (health_cmpnt == NULL){
        printf("Health component failed to allocate\n");
        exit(1);
    }

    HealthManager *health_mgn = mem_sys_alloc(sizeof(HealthManager));
    if (health_mgn == NULL){
        printf("Health manager as part of health component failed to allocate\n");
        exit(1);
    }

    /** Populate the component */
    health_cmpnt->cmpnt_type = CMPNT_HEALTH;
    health_cmpnt->health_mng = health_mgn;
    health_cmpnt->init = init_health_cmpnt;
    health_cmpnt->tick = health_cmpnt_tick;
    health_cmpnt->shutdown = NULL;

    /** Log it to the scheduler */
    schdlr_sys_log_cmpnt(health_cmpnt);

    printf("[HEALTH COMPONENT] - Alive\n");
}

void health_cmpnt_tick(Component *health_cmpnt)
{

    if (health_cmpnt == NULL){
        printf("[HEALTH COMPONENT] - NULL\n");
        return;
    }
    if (health_cmpnt->cmpnt_type != CMPNT_HEALTH){
        printf("[HEALTH COMPONENT] - Wrong component has reached Health Component Tick function\n");
        exit(1);
    }
    printf("[HEALTH COMPONENT] - Health status: %hu\n", health_cmpnt->health_mng->state);
}