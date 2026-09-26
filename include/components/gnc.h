

#ifndef GNC_H
#define GNC_H


#include "../common/mem_chunk.h"
#include "../common/message.h"
#include "../common/channel.h"


#define GNC_Q_SIZE 3


typedef struct GNC_Packet{
    int dummy;
}GNC_Packet;

typedef struct GNC_Pub{
    MemoryChunk chunk;
    uint8_t raw_data[MAX_COMMAND_PAYLOAD_SIZE];

}GNC_Pub;

void init_gnc_cmpnt();

#endif //GNC_H