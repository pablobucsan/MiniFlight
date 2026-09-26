

#ifndef IMU_DRIVER_H
#define IMU_DRIVER_H


#include "rtu_imu.h"
#include <stdint.h>
#include "../common/fault.h"

/**
 * DOCUMENTATION:
 * Assume virtual microcontroller has up to 
 * 1 (Acceleromater) IMU SENSOR THAT MEASURES LINEAR ACCELERATION -> CHANNEL #0
 * 1 (Gyroscope) IMU SENSOR THAT MEASURES ROTATION RATE ()        -> CHANNEL #1
 * 
 * 
 * 
 * Rule of thumb: the spacecraft case is basically the same as the 
 * microcontroller one, just that instead of handling every sensor individually 
 * via its GPIO, you handle a bundle of them by category and read the message 
 * packet formed by all the values read
 */

 #define IMU_INT_TO_FLOAT 0.0625

typedef struct MemoryChunk MemoryChunk;



typedef struct IMU_Packet{
    float accelerometer[3];
    float gyroscope[3];
}IMU_Packet;


AcquireState imu_driver_read(MemoryChunk *imu_chunk);


#endif //IMU_DRIVER_H