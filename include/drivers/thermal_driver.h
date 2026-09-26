

#ifndef THERMAL_DRIVER_H
#define THERMAL_DRIVER_H

#include "rtu_thermal.h"
#include "../common/fault.h"
/**
 * DOCUMENTATION:
 * Assume virtual microcontroller has up to 
 * 1 TEMPERATURE SENSOR SOMEWHERE IN THE VEHICLE            -> CHANNEL #0
 * 1 TEMPERATURE SENSOR SOMEWHERE ELSE IN THE VEHICLE       -> CHANNEL #1
 * 
 * 
 */

#define THERMAL_INT_TO_FLOAT 0.0625

typedef struct MemoryChunk MemoryChunk;


typedef struct Thermal_Packet{
    float temp_1;
    float temp_2;
}Thermal_Packet;


AcquireState thermal_driver_read(MemoryChunk *thermal_chunk);

#endif //THERMAL_DRIVER_H