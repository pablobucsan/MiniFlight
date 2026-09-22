

#ifndef RTU_THERMAL_H
#define RTU_THERMAL_H

#include <stdint.h>



/** 
 * A SIMULATED RTU PACKET FOR THE THERMAL UNIT
 * 
 */
typedef struct RTU_Thermal_Packet{
    uint16_t temp_1;
    uint16_t temp_2;
}RTU_Thermal_Packet;


#endif //RTU_THERMAL_H