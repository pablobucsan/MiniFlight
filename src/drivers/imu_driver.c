

#include "../../include/drivers/imu_driver.h"
#include "../../include/drivers/rtu_imu.h"
#include "../../include/system/memory.h"
#include "../../include/common/mem_chunk.h"

#include <stdlib.h>
#include <stdio.h>


/** Static memory location simulating the RTU IMU reading location */
RTU_IMU_Packet rtu_imu_packet;


RTU_IMU_Packet *rtu_imu_read()
{
    rtu_imu_packet.accelerometer[0] = 0;
    rtu_imu_packet.accelerometer[1] = 1;
    rtu_imu_packet.accelerometer[2] = 2;

    rtu_imu_packet.gyroscope[0] = 0;
    rtu_imu_packet.gyroscope[1] = 1;
    rtu_imu_packet.gyroscope[2] = 2;

    return &rtu_imu_packet;

}


void imu_driver_read(MemoryChunk *imu_chunk)
{
    /** Read the latest value from the RTU 
     *  In this simulation case we just generate one
     */

    RTU_IMU_Packet *rtu_imu_packet = rtu_imu_read();

    IMU_Packet imu_packet;

    /** Process the data converting integers to floats 
     * according to the specific sensor datasheet. In this case
     * we just multiply by a fixed value
    */
    
    for (int i = 0; i < 3; i++){
        imu_packet.accelerometer[i] = rtu_imu_packet->accelerometer[i] * IMU_INT_TO_FLOAT;
        imu_packet.gyroscope[i] = rtu_imu_packet->gyroscope[i] * IMU_INT_TO_FLOAT;
    }

    size_t bytes_written = mem_chunk_write(imu_chunk, (uint8_t *)&imu_packet, sizeof(IMU_Packet));
    if (bytes_written != sizeof(IMU_Packet)){
        printf("[IMU DRIVER] - IMU Packet writing to chunk got truncated\n");
        exit(1);
    }
}
