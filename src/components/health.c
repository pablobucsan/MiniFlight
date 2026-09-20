


#include "../../include/components/health.h"
#include "../../include/system/memory.h"
#include "../../include/system/scheduler.h"
#include "../../include/common/comps.h"
#include <stdio.h>
#include <stdlib.h>


/** Forward declaration of functions to reference them */
void init_health_cmpnt(SchedulerSystem *schdlr_sys);
void health_cmpnt_tick();

static Component *health_cmpnt = NULL;

void init_health_cmpnt(SchedulerSystem *schdlr_sys)
{
    health_cmpnt = mem_sys_alloc(sizeof(Component));
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
    schdlr_sys_log_cmpnt(schdlr_sys, health_cmpnt);

    printf("[HEALTH COMPONENT] - Alive\n");
}

void health_cmpnt_tick()
{
    if (health_cmpnt == NULL){
        printf("[HEALTH COMPONENT] - NULL\n");
        return;
    }
    printf("[HEALTH COMPONENT] - Health status: %hu\n", health_cmpnt->health_mng->state);
}