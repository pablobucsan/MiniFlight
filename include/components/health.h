

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
typedef struct MemoryBuffer MemoryBuffer;

typedef enum HealthState{
    HEALTH_NOMINAL,
    HEALTH_DEGRADED,
    HEALTH_SAFE
}HealthState;

typedef struct Health_Packet{
    HealthState imu_state;
    HealthState thermal_state;
}Health_Packet;

typedef struct HealthManager{
    int sqn_number;
    MemoryBuffer *health_packet_buffer;
}HealthManager;


void init_health_cmpnt();

#endif //HEALTH_H
