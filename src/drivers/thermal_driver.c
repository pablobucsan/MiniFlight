

#include "../../include/drivers/thermal_driver.h"
#include "../../include/system/memory.h"


#include <stdlib.h>
#include <stdio.h>

RTU_Thermal_Packet rtu_thermal_packet;

RTU_Thermal_Packet *rtu_thermal_read()
{

    rtu_thermal_packet.temp_1 = 20;
    rtu_thermal_packet.temp_2 = 40;

    return &rtu_thermal_packet;

}


Thermal_Packet *thermal_driver_read(MemoryBuffer *thermal_packet_buffer)
{
    /** Read the latest value from the RTU 
     *  In this simulation case we just generate one
     */

    RTU_Thermal_Packet *rtu_thermal_packet = rtu_thermal_read();


    Thermal_Packet *thermal_packet = (Thermal_Packet *)thermal_packet_buffer->buffer;
    if (thermal_packet == NULL){
        printf("[THERMAL DRIVER] - Failed to allocate Thermal Packet\n");
        exit(1);
    }
    
    /** Process the data converting integers to floats 
     * according to the specific sensor datasheet. In this case
     * we just multiply by a fixed value
    */
    
    thermal_packet->temp_1 = rtu_thermal_packet->temp_1 * THERMAL_INT_TO_FLOAT;
    thermal_packet->temp_2 = rtu_thermal_packet->temp_2 * THERMAL_INT_TO_FLOAT;

    return thermal_packet;
}
