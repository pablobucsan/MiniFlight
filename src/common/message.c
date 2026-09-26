

#include "../../include/common/message.h"
#include "../../include/common/subscribers.h"
#include "../../include/system/memory.h"
#include <stdio.h>

/**
 * 
 * @brief Reads the payload of a Message Packet, writing its content into ```out_stream```
 * 
 * @param msg_packet A ```NON-NULL``` pointer to the Message Packet whose payload we want to read
 * @param out_stream A ```NON-NULL``` pointer to the stream in which the Message Packet payload will be written into
 * @param intended_bytes_to_read Number of bytes we intent to read from the payload
 * 
 * @returns The actual bytes read from the payload, capping it at the payload length
 */
size_t msg_pkt_read_payload(Msg_Packet *msg_packet, uint8_t *out_stream, size_t intended_bytes_to_read)
{

    size_t actual_bytes_to_read = intended_bytes_to_read;

    if (intended_bytes_to_read > msg_packet->header.length){
        actual_bytes_to_read = msg_packet->header.length;
    }
    mem_sys_copy(out_stream, msg_packet->payload, actual_bytes_to_read);
    return actual_bytes_to_read;
}