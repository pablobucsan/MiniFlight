

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

#define HEALTH_Q_SIZE 3

#include "../common/message.h"
#include "../common/mem_chunk.h"
#include "../common/channel.h"
#include <stdint.h>

typedef struct SchedulerSystem SchedulerSystem;

typedef enum HealthState{
    HEALTH_NOMINAL,
    HEALTH_DEGRADED,
    HEALTH_SAFE
}HealthState;

typedef struct Health_Packet{
    HealthState imu_state;
    HealthState thermal_state;
}Health_Packet;

typedef struct Health_Pub{
    MemoryChunk chunk;
    uint8_t raw_data[MAX_TELEMETRY_PAYLOAD_SIZE];

    Channel status_channel;
}Health_Pub;


void init_health_cmpnt();

#endif //HEALTH_H
