

#include "../../include/drivers/thermal_driver.h"
#include "../../include/system/memory.h"
#include "../../include/common/mem_chunk.h"

#include <stdlib.h>
#include <stdio.h>



static uint16_t max_temp = 100;

/**
 * @brief Simulates the RTU Thermal memory location that may be accessed to at a known 
 * fixed memory location in real scenarios
 */
RTU_Thermal_Packet rtu_thermal_packet;

/**
 * @brief File-private helper that simulates reading from the RTU Thermal memory location
 */
static RTU_Thermal_Packet *rtu_thermal_read()
{

    rtu_thermal_packet.temp_1 = rand() % max_temp;
    rtu_thermal_packet.temp_2 = rand() % max_temp;

    return &rtu_thermal_packet;

}


/**
 * @brief Orchestrates RTU Thermal data reading and writes it into the ```thermal_chunk``` buffer
 * 
 * @warning 
 * - On writing failure, proceeds to ```exit()```
 * 
 * @param thermal_chunk The handed-off chunk to write into
 * 
 * @return An ```AcquiteState``` enum value
 * 
 * - ```ACQUIRE_GOOD``` if Thermal_Packet has been written healthy to the chunk
 * - ```ACQUIRE_BAD``` if Thermal_Packet has not been written healthy to the chunk
 */
AcquireState thermal_driver_read(MemoryChunk *thermal_chunk)
{
    /** Read the latest value from the RTU 
     *  In this simulation case we just generate one
     */

    RTU_Thermal_Packet *rtu_thermal_packet = rtu_thermal_read();


    Thermal_Packet thermal_packet;

    /** If failed to read RTU Thermal packet */
    if (rtu_thermal_packet == NULL){
        printf("[THERMAL DRIVER] - Failed to read RTU Thermal packet\n");
        return ACQUIRE_BAD;
    }
    /** Check that the values are within range */
    if (rtu_thermal_packet->temp_1 > max_temp || 
        rtu_thermal_packet->temp_2 > max_temp){
            printf("[THERMAL DRIVER] - Sensor values out of range\n");
            return ACQUIRE_BAD;
    }
    /** Process the data converting integers to floats 
     * according to the specific sensor datasheet. In this case
     * we just multiply by a fixed value
    */
    
    thermal_packet.temp_1 = rtu_thermal_packet->temp_1 * THERMAL_INT_TO_FLOAT;
    thermal_packet.temp_2 = rtu_thermal_packet->temp_2 * THERMAL_INT_TO_FLOAT;

    size_t bytes_written = mem_chunk_write(thermal_chunk, (uint8_t *)&thermal_packet, sizeof(Thermal_Packet));
    if (bytes_written != sizeof(Thermal_Packet)){
        printf("[THERMAL DRIVER] - Thermal Packet writing to chunk got truncated\n");
        return ACQUIRE_BAD;
    }

    return ACQUIRE_GOOD;
}
