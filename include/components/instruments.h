

#ifndef INSTRUMENTS_H
#define INSTRUMENTS_H

/**
 * DESCRIPTION: High-level abstraction and coordination of instruments
 * 
 * RESPONSIBILITIES: 
 * Translate bus messages into driver-level requests
 * Aggregate data from multiple instruments when needed
 * Publish intrument data and status onto the Software Bus
 */


typedef struct MemoryBuffer MemoryBuffer;


typedef struct InstrumentsManager{
    MemoryBuffer *imu_packet_buffer;
    MemoryBuffer *thermal_packet_buffer;
    int imu_sqn_number;
    int thermal_sqn_number;
}InstrumentsManager;

void init_instruments_cmpnt();


#endif //INSTRUMENTS_MANAGER_H