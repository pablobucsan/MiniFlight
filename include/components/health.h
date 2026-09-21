

#ifndef HEALTH_H
#define HEALTH_H

/**
 * DESCRIPTION: System health monitoring and basic fault awareness
 * 
 * RESPONSIBILITIES: 
 * Subscribe to heartbeat/status messages from other components
 * Detect missing heartbeats or explicit error flags
 * Maintain an overall system health state (nominal / degraded / safe)
 * Publish health status and alerts onto the Software Bus
 * Optionally trigger a simple safe-mode reaction (inhibit actuators)
 */

typedef struct SchedulerSystem SchedulerSystem;

typedef enum HealthState{
    HEALTH_NOMINAL,
    HEALTH_DEGRADED,
    HEALTH_SAFE
}HealthState;

typedef struct HealthManager{
    HealthState state;
}HealthManager;


void init_health_cmpnt();
void health_cmpnt_tick();

#endif //HEALTH_H
