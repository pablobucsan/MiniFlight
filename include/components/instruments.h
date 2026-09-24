

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

#include "../common/message.h"
#include "../common/mem_chunk.h"

#define INSTRMNT_Q_SIZE 3



typedef struct InstrumentsManager{
    MemoryChunk chunk;
    uint8_t raw_data[MAX_TELEMETRY_PAYLOAD_SIZE];
    int imu_sqn_number;
    int thermal_sqn_number;
}InstrumentsManager;

void init_instruments_cmpnt();


#endif //INSTRUMENTS_MANAGER_H