

#include "../../include/drivers/imu_driver.h"
#include "../../include/drivers/rtu_imu.h"
#include "../../include/system/memory.h"

#include <stdlib.h>
#include <stdio.h>

RTU_IMU_Packet *rtu_imu_read()
{
    /** This wouldn't generally be allocated but 
     * read from a set memory location */
    RTU_IMU_Packet *rtu_imu_packet = mem_sys_alloc(sizeof(RTU_IMU_Packet));
    if (rtu_imu_packet == NULL){
        printf("[IMU DRIVER] - Failed to allocate space for creating RTU IMU packet\n");
        exit(1);
    }

    rtu_imu_packet->accelerometer[0] = 0;
    rtu_imu_packet->accelerometer[1] = 1;
    rtu_imu_packet->accelerometer[2] = 2;

    rtu_imu_packet->gyroscope[0] = 0;
    rtu_imu_packet->gyroscope[1] = 1;
    rtu_imu_packet->gyroscope[2] = 2;

    return rtu_imu_packet;

}


IMU_Packet *imu_driver_read()
{
    /** Read the latest value from the RTU 
     *  In this simulation case we just generate one
     */

    RTU_IMU_Packet *rtu_imu_packet = rtu_imu_read();


    IMU_Packet *imu_packet = mem_sys_alloc(sizeof(IMU_Packet));
    if (imu_packet == NULL){
        printf("[IMU DRIVER] - Failed to allocate IMU Packet\n");
        exit(1);
    }
    
    /** Process the data converting integers to floats 
     * according to the specific sensor datasheet. In this case
     * we just multiply by a fixed value
    */
    
    for (int i = 0; i < 3; i++){
        imu_packet->accelerometer[i] = rtu_imu_packet->accelerometer[i] * IMU_INT_TO_FLOAT;
        imu_packet->gyroscope[i] = rtu_imu_packet->gyroscope[i] * IMU_INT_TO_FLOAT;
    }

    return imu_packet;
}
