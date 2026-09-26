

#ifndef RTU_IMU_H
#define RTU_IMU_H

#include <stdint.h>


/** 
 * @brief A simulated RTU packet for the IMU unit
 * 
 */
typedef struct RTU_IMU_Packet{
    uint16_t accelerometer[3];
    uint16_t gyroscope[3];
}RTU_IMU_Packet;


#endif //RTU_IMU_H