

#include "../../include/drivers/imu_driver.h"
#include "../../include/drivers/rtu_imu.h"
#include "../../include/system/memory.h"
#include "../../include/common/mem_chunk.h"

#include <stdlib.h>
#include <stdio.h>

static uint16_t max_acc = 100;
static uint16_t max_gyro = 100;

/**
 * @brief Simulates the RTU IMU memory location that may be accessed to at a known 
 * fixed memory location in real scenarios
 */
RTU_IMU_Packet rtu_imu_packet;

/**
 * @brief File-private helper that simulates reading from the RTU IMU memory location
 */
static RTU_IMU_Packet *rtu_imu_read()
{

    uint16_t acc = rand() % max_acc;
    uint16_t gyro = rand() % max_gyro;


    rtu_imu_packet.accelerometer[0] = acc;
    rtu_imu_packet.accelerometer[1] = acc;
    rtu_imu_packet.accelerometer[2] = acc;

    rtu_imu_packet.gyroscope[0] = gyro;
    rtu_imu_packet.gyroscope[1] = gyro;
    rtu_imu_packet.gyroscope[2] = gyro;

    return &rtu_imu_packet;

}

/**
 * @brief Orchestrates RTU IMU data reading and writes it into ```imu_chunk``` buffer
 * 
 * 
 * @param imu_chunk The handed-off chunk to write into
 * 
 * @return An ```AcquireState``` enum value.
 * 
 * - ```ACQUIRE_GOOD``` if IMU_Packet has been written healthy to the chunk
 * - ```ACQUIRE_BAD``` if IMU_Packet has not been written healthy to the chunk
 */
AcquireState imu_driver_read(MemoryChunk *imu_chunk)
{
    /** Read the latest value from the RTU 
     *  In this simulation case we just generate one
     */

    RTU_IMU_Packet *rtu_imu_packet = rtu_imu_read();

    IMU_Packet imu_packet;

    /** If failed to read RTU IMU packet */
    if (rtu_imu_packet == NULL){
        printf("[IMU DRIVER] - Failed to read RTU IMU packet\n");
        return ACQUIRE_BAD;
    }

    /** Process the data converting integers to floats 
     * according to the specific sensor datasheet. In this case
     * we just multiply by a fixed value
    */
    
    for (int i = 0; i < 3; i++){
        /** Check that the values are within range */
        if (rtu_imu_packet->accelerometer[i] > max_acc || 
            rtu_imu_packet->gyroscope[i] > max_gyro){
                printf("[IMU DRIVER] - Sensor values out of range\n");
                return ACQUIRE_BAD;
        }

        /** Convert to float */
        imu_packet.accelerometer[i] = rtu_imu_packet->accelerometer[i] * IMU_INT_TO_FLOAT;
        imu_packet.gyroscope[i] = rtu_imu_packet->gyroscope[i] * IMU_INT_TO_FLOAT;
    }

    size_t bytes_written = mem_chunk_write(imu_chunk, (uint8_t *)&imu_packet, sizeof(IMU_Packet));
    if (bytes_written != sizeof(IMU_Packet)){
        printf("[IMU DRIVER] - IMU Packet writing to chunk got truncated\n");
        return ACQUIRE_BAD;
    }

    return ACQUIRE_GOOD;
}

