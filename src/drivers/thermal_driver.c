

#include "../../include/drivers/thermal_driver.h"
#include "../../include/system/memory.h"
#include "../../include/common/mem_chunk.h"

#include <stdlib.h>
#include <stdio.h>

RTU_Thermal_Packet rtu_thermal_packet;

RTU_Thermal_Packet *rtu_thermal_read()
{

    rtu_thermal_packet.temp_1 = 20;
    rtu_thermal_packet.temp_2 = 40;

    return &rtu_thermal_packet;

}


void thermal_driver_read(MemoryChunk *thermal_chunk)
{
    /** Read the latest value from the RTU 
     *  In this simulation case we just generate one
     */

    RTU_Thermal_Packet *rtu_thermal_packet = rtu_thermal_read();


    Thermal_Packet thermal_packet;
    
    /** Process the data converting integers to floats 
     * according to the specific sensor datasheet. In this case
     * we just multiply by a fixed value
    */
    
    thermal_packet.temp_1 = rtu_thermal_packet->temp_1 * THERMAL_INT_TO_FLOAT;
    thermal_packet.temp_2 = rtu_thermal_packet->temp_2 * THERMAL_INT_TO_FLOAT;

    size_t bytes_written = mem_chunk_write(thermal_chunk, (uint8_t *)&thermal_packet, sizeof(Thermal_Packet));
    if (bytes_written != sizeof(Thermal_Packet)){
        printf("[THERMAL DRIVER] - Thermal Packet writing to chunk got truncated\n");
        exit(1);
    }
}
