#ifndef MESSAGE_H
#define MESSAGE_H

#include <stdint.h>


#define MAX_SQN_NUMBER 32

#define MAX_TELEMETRY_PAYLOAD_SIZE 512


/**
 * This would the max of all the msg type sizes
 */
#define MAX_MSG_PACKET_SIZE 1024

typedef enum MessageID{
    MSG_ID_NONE = 0x0,
    MSG_ID_HEALTH_INSTRMNT = 0x1,
    MSG_ID_INSTRMNT_IMU = 0x2,
    MSG_ID_INSTRMNT_THERMAL = 0x3,
}MessageID;

typedef enum Msg_Type{
    MSG_TYPE_NONE,
    MSG_TYPE_TELEMETRY,
    MSG_TYPE_CMD
}Msg_Type;


/**
 *                   Message structure
 * 
 * 
 * --------------------------------------------------------------------
 * |              Type (1 bit) - Telemetry or Command   (2 types)     |
 * |------------------------------------------------------------------|
 * |              Message ID (3 bits) - Routing key   (8 IDs)         |
 * |------------------------------------------------------------------|                        
 * |    App ID (4 bits) - Identifier of the publisher cmpnt (16 IDs)  |
 * |------------------------------------------------------------------|
 * |   Timestamp & Sequence number (per-source / per- topic counter)  |
 * |       (16 bits) - Helps detect gaps or reordering during demos   |
 * |------------------------------------------------------------------|
 * |                 Length (16 bits) - Size of the payload           |
 * |------------------------------------------------------------------|
 * | Command field (4 bits) - Action specifier for cmd messages       |
 * |                          (16 cmds)                                |
 * |------------------------------------------------------------------|
 * |                                                                  |
 * |                                                                  |
 * |                                                                  |
 * |                                                                  |
 * |                      Payload (variable)                          |
 * |                                                                  |
 * |                                                                  |
 * |                                                                  |
 * |                                                                  |
 * --------------------------------------------------------------------
 * 
 */

/** 
 * 
 * 
 */

typedef struct Msg_Packet_H{
    uint16_t length;
    uint16_t sqn_number;
    uint8_t type;
    uint8_t msg_id;
    uint8_t cmpnt_id;
    uint8_t cmd;
}Msg_Packet_H;


typedef struct Msg_Packet{
    Msg_Packet_H header;
    uint8_t payload[MAX_MSG_PACKET_SIZE];
}Msg_Packet;


#endif //MESSAGE_H