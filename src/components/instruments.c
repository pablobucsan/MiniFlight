

#include "../../include/components/instruments.h"
#include "../../include/system/memory.h"
#include "../../include/system/scheduler.h"
#include "../../include/common/subscribers.h"
#include "../../include/common/comps.h"
#include "../../include/bus/sw_bus.h"
#include "../../include/drivers/imu_driver.h"
#include "../../include/drivers/thermal_driver.h"
#include "../../include/components/health.h"
#include "../../include/common/message.h"
#include <assert.h>


#include <stdio.h>
#include <stdlib.h>



/**
 * 
 * 
 * File-private static singletons
 */

static Component instruments_cmpnt;
static InstrumentsManager instruments_mng;



/**
 * Internal private helper functions
 */
static void instruments_cmpnt_tick(Component *instruments_cmpnt);
static void instruments_cmpnt_on_msg_received(Component *cmpnt, Msg_Packet *msg_packet);

static void instruments_cmpnt_handle_health_instrmt_msg(Component *cmpnt, Health_Packet *health_packet);
static void instruments_cmpnt_msg_task(Component *instruments_cmpnt);
static void instruments_cmpnt_imu_task(Component *instruments_cmpnt);
static void instruments_cmpnt_thermal_task(Component *instruments_cmpnt);
static void instruments_cmpnt_subify(Component *instruments_cmpnt);

/**
 * 
 * Static assert ot guarantee that IMU Packet and Thermal Packet will
 * always fit inside of Instrument Manager
 */

static_assert(sizeof(IMU_Packet) <= MAX_TELEMETRY_PAYLOAD_SIZE,
    "IMU_Packet exceeds maximum allocated telemetry buffer capacity\n");

static_assert(sizeof(Thermal_Packet) <= MAX_TELEMETRY_PAYLOAD_SIZE,
    "Thermal_Packet exceeds maximum allocated telemetry buffer capacity\n");





static void instruments_cmpnt_subify(Component *instruments_cmpnt)
{
    /** Make the subscriber and link to the component */
    Subscriber *instruments_sub = make_subscriber(INSTRMNT_Q_SIZE, Q_DROP_OLDEST);
    instruments_cmpnt->subscriber = instruments_sub;
    /** Add its subcription to IDs */
    subscriber_sub_to_msg_id(instruments_sub, MSG_ID_HEALTH_INSTRMNT);

    /** Log to the Software Bus table */
    swbus_log_subscriber(instruments_sub);

}

void init_instruments_cmpnt()
{
    /** Populate the manager */

    size_t data_capacity = sizeof(IMU_Packet) + sizeof(Thermal_Packet);
    instruments_mng.imu_sqn_number = 0;
    instruments_mng.thermal_sqn_number = 0;
    instruments_mng.chunk = mem_chunk_init(instruments_mng.raw_data,data_capacity);



    /** Populate the component */
    instruments_cmpnt.cmpnt_id = CMPNT_ID_INSTRUMENTS;
    instruments_cmpnt.mng = &instruments_mng;
    instruments_cmpnt.tick = instruments_cmpnt_tick;
    instruments_cmpnt.on_msg_received = instruments_cmpnt_on_msg_received;

    /** Log it to the Scheduler */
    schdlr_sys_log_cmpnt(&instruments_cmpnt);

    /** Log it to the Software Bus table */
    instruments_cmpnt_subify(&instruments_cmpnt);
}

void instruments_cmpnt_tick(Component *instruments_cmpnt)
{

    printf("[INSTRUMENTS COMPONENT] - TICKING...\n");

    if (instruments_cmpnt->cmpnt_id != CMPNT_ID_INSTRUMENTS){
        printf("[INSTRUMENTS COMPONENT] - Wrong component has reached Instrument Component Tick function\n");
        exit(1);
    }

    /** Handle incoming messages */
    instruments_cmpnt_msg_task(instruments_cmpnt);
    /**
     * Rest of the tick() function.
     */

    printf("[INSTRUMENTS COMPONENT] - Finished msg task\n");
     /** IMU Task */
    instruments_cmpnt_imu_task(instruments_cmpnt);
    printf("[INSTRUMENTS COMPONENT] - Finished imu task\n");
    /** Thermal Task */
    instruments_cmpnt_thermal_task(instruments_cmpnt);

    printf("[INSTRUMENTS COMPONENT] - TICKED\n");

}


static void instruments_cmpnt_msg_task(Component *instruments_cmpnt)
{
    /** Handle incoming messages */
    if (instruments_cmpnt->subscriber == NULL){
        return;
    }
    /**
     * Should tick() act on a reference to one of the messages the subscriber queue contains
     * or instead copy it locally to the stack? 
     * 
     * Better to copy to the stack so if another message comes from the bus and takes that very slot,
     * we are safe
     */
    Msg_Packet msg_packet;
    while (subscriber_dequeue_msg(instruments_cmpnt->subscriber, &msg_packet)){
        instruments_cmpnt_on_msg_received(instruments_cmpnt, &msg_packet);
    }
    
}


static void instruments_cmpnt_imu_task(Component *instruments_cmpnt)
{
    /**
     * 
     * Rest of the tick() function.
     */

    InstrumentsManager *instruments_mng = (InstrumentsManager *)instruments_cmpnt->mng;
    /** Perform IMU reading, the driver will write into the handed off chunk */
    imu_driver_read(&instruments_mng->chunk);

    /** Request a memory chunk from the bus to form the message packet */
    size_t total_msg_size = sizeof(Msg_Packet_H) + sizeof(IMU_Packet);
    MemoryChunk *msg_packet_chunk = swbus_rqst_mem_chunk(total_msg_size);

    if (msg_packet_chunk == NULL){
        printf("[INSTRUMENTS COMPONENT] - No memory chunk available to publish IMU\n");
        return;
    }

    /** Create the header for the message packet */
    Msg_Packet_H msg_pkt_h = {
        .length = sizeof(IMU_Packet),
        .sqn_number = instruments_mng->imu_sqn_number,
        .type = MSG_TYPE_TELEMETRY,
        .msg_id = MSG_ID_INSTRMNT_IMU,
        .cmpnt_id = instruments_cmpnt->cmpnt_id,
        .cmd = 0,
    };

    /** Update sequence number */
    instruments_mng->imu_sqn_number++;
    if (instruments_mng->imu_sqn_number >= MAX_SQN_NUMBER){
        instruments_mng->imu_sqn_number = 0;
    }

    /** Write the header into the Message Packet chunk */
    size_t header_bytes_written = mem_chunk_write(msg_packet_chunk, (uint8_t *)&msg_pkt_h, sizeof(Msg_Packet_H));
    if (header_bytes_written != sizeof(Msg_Packet_H)){
        printf("[INSTRUMENTS COMPONENT] - Header writing to msg packet got truncated\n");
        exit(1);
    }

    /** Write the payload into the Message Packet chunk */
    size_t payload_bytes_written = mem_chunk_write(msg_packet_chunk, (uint8_t *)instruments_mng->raw_data, sizeof(IMU_Packet));
    if (payload_bytes_written != sizeof(IMU_Packet)){
        printf("[INSTRUMENTS COMPONENT] - Payload writing to msg packet got truncated\n");
        exit(1);
    }


    /** Instrument Manager owned memory is now safe to reuse */
    mem_chunk_clear(&instruments_mng->chunk);

    printf("[INSTRUMENTS COMPONENT] - About to publish IMU\n");
    swbus_publish(msg_packet_chunk, msg_pkt_h.msg_id);


}


static void instruments_cmpnt_thermal_task(Component *instruments_cmpnt)
{
    /**
     * 
     * Rest of the tick() function.
     */

    InstrumentsManager *instruments_mng = (InstrumentsManager *)instruments_cmpnt->mng;

    /** Perform IMU reading, the driver will write into the handed off chunk */
    thermal_driver_read(&instruments_mng->chunk);

    /** Request a memory chunk from the bus to form the message packet */
    size_t total_msg_size = sizeof(Msg_Packet_H) + sizeof(Thermal_Packet);
    MemoryChunk *msg_packet_chunk = swbus_rqst_mem_chunk(total_msg_size);

    if (msg_packet_chunk == NULL){
        printf("[INSTRUMENTS COMPONENT] - No memory chunk available to publish Thermal\n");
        return;
    }

    /** Create the header for the message packet */
    Msg_Packet_H msg_pkt_h = {
        .length = sizeof(Thermal_Packet),
        .sqn_number = instruments_mng->thermal_sqn_number,
        .type = MSG_TYPE_TELEMETRY,
        .msg_id = MSG_ID_INSTRMNT_THERMAL,
        .cmpnt_id = instruments_cmpnt->cmpnt_id,
        .cmd = 0,
    };

    /** Update sequence number */
    instruments_mng->thermal_sqn_number++;
    if (instruments_mng->thermal_sqn_number >= MAX_SQN_NUMBER){
        instruments_mng->thermal_sqn_number = 0;
    }

    /** Write the header into the Message Packet chunk */
    size_t header_bytes_written = mem_chunk_write(msg_packet_chunk, (uint8_t *)&msg_pkt_h, sizeof(Msg_Packet_H));
    if (header_bytes_written != sizeof(Msg_Packet_H)){
        printf("[INSTRUMENTS COMPONENT] - Header writing to msg packet got truncated\n");
        exit(1);
    }

    /** Write the payload into the Message Packet chunk */
    size_t payload_bytes_written = mem_chunk_write(msg_packet_chunk, (uint8_t *)instruments_mng->raw_data, sizeof(Thermal_Packet));
    if (payload_bytes_written != sizeof(Thermal_Packet)){
        printf("[INSTRUMENTS COMPONENT] - Payload writing to msg packet got truncated\n");
        exit(1);
    }

    /** Instrument Manager owned memory is now safe to reuse */
    mem_chunk_clear(&instruments_mng->chunk);

    swbus_publish(msg_packet_chunk, msg_pkt_h.msg_id);


}



static void instruments_cmpnt_on_msg_received(Component *cmpnt, Msg_Packet *msg_packet)
{
    if (msg_packet == NULL){
        printf("[INSTRUMENTS COMPONENT ON MSG RECEIVED] - Received NULL message packet\n");
        return;
    }

    MessageID msg_id = msg_packet->header.msg_id;
    switch(msg_id){
        case MSG_ID_HEALTH_INSTRMNT:{
            Health_Packet *health_packet = (Health_Packet *)msg_packet->payload;
            instruments_cmpnt_handle_health_instrmt_msg(cmpnt, health_packet);
            break;
        }

        default:{
            printf("[INSTRUMENTS COMPONENT ON MSG RECEIVED] - Unknown msg id to handle\n");
            break;
        }
    }

}

static void instruments_cmpnt_handle_health_instrmt_msg(Component *cmpnt, Health_Packet *health_packet)
{
    if (health_packet == NULL){
        printf("[INSTRUMENTS COMPONENT] - Received NULL Health Packet\n");
        return;
    }

    /** Here we could perform actions based on the health of every instrument unit */
    printf("=======================\n");
    printf("[INSTRUMENTS COMPONENT ON MSG RECEIVED] - IMU healh status: %hu\n", health_packet->imu_state);
    printf("[INSTRUMENTS COMPONENT ON MSG RECEIVED] - Thermal healh status: %hu\n", health_packet->thermal_state);
    printf("=======================\n");

}



