#ifndef MESSAGE_H
#define MESSAGE_H


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
 * | Command field (3 bits) - Action specifier for cmd messages       |
 * |                          (8 cmds)                                |
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


#endif //MESSAGE_H