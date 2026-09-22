

#ifndef RTU_IMU_H
#define RTU_IMU_H

#include <stdint.h>


/** 
 * A SIMULATED RTU PACKET FOR THE IMU UNIT
 * 
 */
typedef struct RTU_IMU_Packet{
    uint16_t accelerometer[3];
    uint16_t gyroscope[3];
}RTU_IMU_Packet;


#endif //RTU_IMU_H