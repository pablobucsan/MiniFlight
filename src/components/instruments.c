

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
#include "../../include/common/channel.h"
#include "../../include/common/math.h"
#include <assert.h>


#include <stdio.h>
#include <stdlib.h>



/**
 * @brief File-private Instrument component singleton
 */
static Component instruments_cmpnt;
/**
 * @brief File-private Instrument publisher singleton
 */
static Instruments_Pub instruments_pub;


/**
 * Internal private helper functions
 */


static void instruments_cmpnt_tick(Component *instruments_cmpnt);
static void instruments_cmpnt_on_msg_received(Component *cmpnt, Msg_Packet *msg_packet);

static void instruments_cmpnt_msg_task(Component *instruments_cmpnt);
static void instruments_cmpnt_imu_task(Component *instruments_cmpnt);
static void instruments_cmpnt_thermal_task(Component *instruments_cmpnt);

static void instruments_cmpnt_handle_MSG_ID_HEALTH_INSTRMNT(Component *instruments_cmpnt, Health_Packet *health_packet);


/**
 * 
 * Static assert ot guarantee that IMU Packet and Thermal Packet will
 * always fit inside of Instrument Manager
 */

static_assert(sizeof(IMU_Packet) <= MAX_TELEMETRY_PAYLOAD_SIZE,
    "IMU_Packet exceeds maximum allocated telemetry buffer capacity\n");

static_assert(sizeof(Thermal_Packet) <= MAX_TELEMETRY_PAYLOAD_SIZE,
    "Thermal_Packet exceeds maximum allocated telemetry buffer capacity\n");


/**
 * @brief Actions:
 * 
 * - Initializes the Instruments Manager, its fields and chunk
 * 
 * - Initializes the Instruments Component, its fields, links to the manager and to its ```tick()``` function
 * 
 * - Logs the Instruments Component to the Scheduler's Component table
 * 
 * - Subifies the Instrument Component, populating its ```.subcriber``` field
 */
void init_instruments_cmpnt()
{
    /** 
     * Set up the publisher, chunk and its production channels
     */
    size_t data_capacity = max_size(sizeof(IMU_Packet), sizeof(Thermal_Packet));
    instruments_pub.chunk = mem_chunk_init(instruments_pub.raw_data,data_capacity);
    instruments_pub.imu_channel = channel_init(CHANNEL_PRODUCTION);
    instruments_pub.thermal_channel = channel_init(CHANNEL_PRODUCTION);

    /** Set up the subscriber, sub to IDs and log it to the Software Bus  */
    Subscriber *instruments_sub = make_subscriber(INSTRMNT_Q_SIZE, Q_DROP_OLDEST);
    subscriber_sub_to_msg_id(instruments_sub, MSG_ID_HEALTH_INSTRMNT);
    swbus_log_subscriber(instruments_sub);

    /** Set up the component */
    instruments_cmpnt.cmpnt_id = CMPNT_ID_INSTRUMENTS;
    instruments_cmpnt.publisher = &instruments_pub;
    instruments_cmpnt.tick = instruments_cmpnt_tick;
    instruments_cmpnt.subscriber = instruments_sub;

    /** Log it to the Scheduler */
    schdlr_sys_log_cmpnt(&instruments_cmpnt);

    printf("[INSTRUMENTS COMPONENT] - Alive\n");
}

/**
 * 
 * @brief Execution of the Instrument components periodic tasks, like processing queued messages, performing IMU and Thermal measurements 
 * 
 * @warning 
 * - If a component with an id other than ```CMNT_ID_INSTRUMENTS``` reaches this function, proceeds to ```exit()```
 * 
 * @param instruments_cmpnt A ```NON-NULL``` pointer to the Instrument component
 */
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

/**
 * 
 * @brief Performs the message processing task of the Instruments component
 * 
 * @param instruments_cmpnt A ```NON-NULL``` pointer to the Instrument component
 */
static void instruments_cmpnt_msg_task(Component *instruments_cmpnt)
{
    /** Handle incoming messages */
    if (instruments_cmpnt->subscriber == NULL){
        return;
    }
    /**
     * Better to copy to the stack so if another message comes from the bus and takes that very slot,
     * we are safe
     */
    Msg_Packet msg_packet;
    while (subscriber_dequeue_msg(instruments_cmpnt->subscriber, &msg_packet)){
        instruments_cmpnt_on_msg_received(instruments_cmpnt, &msg_packet);
    }
    
}

/**
 * 
 * @brief Performs the IMU measurement task of the Instruments component. It publishes to the Software Bus a Memory Chunk whose payload is an IMU_Packet
 * 
 * @param instruments_cmpnt A ```NON-NULL``` pointer to the Instrument component
 */
static void instruments_cmpnt_imu_task(Component *instruments_cmpnt)
{

    Instruments_Pub *instruments_pub = (Instruments_Pub *)instruments_cmpnt->publisher;



    /** Perform IMU reading, the driver will write into the handed off chunk */
    AcquireState acquire_state = imu_driver_read(&instruments_pub->chunk);
    Channel *imu_channel = &instruments_pub->imu_channel;
    
    
    /** Check whether acquisition was successfull */
    if (acquire_state == ACQUIRE_BAD){
        printf("[INSTRUMENTS COMPONENT] <FAULT> - IMU task: Failed acquisition\n");
        /** Count the fault */
        imu_channel->prod_lfs.acquire_fault_count++;
        /** Clear the chunk in case something was written to it */
        mem_chunk_clear(&instruments_pub->chunk);
        /** Do not continue, there is no good payload to send */
        return;
    }

    /** Request a memory chunk from the bus to form the message packet */
    size_t total_msg_size = sizeof(Msg_Packet_H) + sizeof(IMU_Packet);
    MemoryChunk *msg_packet_chunk = swbus_rqst_mem_chunk(total_msg_size);

    if (msg_packet_chunk == NULL){
        printf("[INSTRUMENTS COMPONENT] <FAULT> - IMU task: Failed publish resource\n");
        imu_channel->prod_lfs.pub_resource_fault_count++;
        /** Do not continue, nowhere to construct the Message Packet */
        return;
    }


    /** Create the header for the message packet */
    Msg_Packet_H msg_pkt_h = {
        .length = sizeof(IMU_Packet),
        .sqn_number = imu_channel->sqn_number,
        .type = MSG_TYPE_TELEMETRY,
        .msg_id = MSG_ID_INSTRMNT_IMU,
        .cmpnt_id = instruments_cmpnt->cmpnt_id,
        .cmd = 0,
    };


    /** Write the header into the Message Packet chunk */
    size_t header_bytes_written = mem_chunk_write(msg_packet_chunk, (uint8_t *)&msg_pkt_h, sizeof(Msg_Packet_H));
    if (header_bytes_written != sizeof(Msg_Packet_H)){
        printf("[INSTRUMENTS COMPONENT] <FAULT> - IMU task: Failed publish build\n");
        imu_channel->prod_lfs.pub_build_fault_count++;
        /** Release the requested chunk */
        swbus_release_chunk(msg_packet_chunk);
        /** Do not continue, failed to write the Message Packet header  */
        return;
    }

    /** Write the payload into the Message Packet chunk */
    size_t payload_bytes_written = mem_chunk_write(msg_packet_chunk, (uint8_t *)instruments_pub->raw_data, sizeof(IMU_Packet));
    if (payload_bytes_written != sizeof(IMU_Packet)){
        printf("[INSTRUMENTS COMPONENT] <FAULT> - IMU task: Failed publish build\n");
        imu_channel->prod_lfs.pub_build_fault_count++;
        /** Release the requested chunk */
        swbus_release_chunk(msg_packet_chunk);
        /** Do not continue, failed to write the Message Packet payload */
        return;
    }


    /** Update Sequence number and last success frame */
    imu_channel->sqn_number++;
    imu_channel->prod_lfs.last_success_frame = schdlr_sys_current_frame();

    /** Instrument Manager owned memory is now safe to reuse */
    mem_chunk_clear(&instruments_pub->chunk);

    swbus_publish(msg_packet_chunk, msg_pkt_h.msg_id);


}

/**
 * 
 * @brief Performs the Thermal measurement task of the Instruments component. It publishes to the Software Bus a Memory Chunk whose payload is a Thermal_Packet
 * 
 * @param instruments_cmpnt A ```NON-NULL``` pointer to the Instrument component
 */
static void instruments_cmpnt_thermal_task(Component *instruments_cmpnt)
{

    Instruments_Pub *instruments_pub = (Instruments_Pub *)instruments_cmpnt->publisher;

    /** Perform Thermal reading, the driver will write into the handed off chunk */
    AcquireState  acquire_state = thermal_driver_read(&instruments_pub->chunk);
    Channel *thermal_channel = &instruments_pub->thermal_channel;

    /** Check whether acquisition was successfull */
    if (acquire_state == ACQUIRE_BAD){
        printf("[INSTRUMENTS COMPONENT] <FAULT> - Thermal task: Failed acquisition\n");
        /** Count the fault */
        thermal_channel->prod_lfs.acquire_fault_count++;
        /** Clear the chunk in case something was written to it */
        mem_chunk_clear(&instruments_pub->chunk);
        /** Do not continue, there is no good payload to send */
        return;
    }

    /** Request a memory chunk from the bus to form the message packet */
    size_t total_msg_size = sizeof(Msg_Packet_H) + sizeof(Thermal_Packet);
    MemoryChunk *msg_packet_chunk = swbus_rqst_mem_chunk(total_msg_size);

    if (msg_packet_chunk == NULL){
        printf("[INSTRUMENTS COMPONENT] <FAULT> - Thermal task: Failed publish resource\n");
        thermal_channel->prod_lfs.pub_resource_fault_count++;
        /** Do not continue, nowhere to construct the Message Packet */
        return;
    }

    /** Create the header for the message packet */
    Msg_Packet_H msg_pkt_h = {
        .length = sizeof(Thermal_Packet),
        .sqn_number = thermal_channel->sqn_number,
        .type = MSG_TYPE_TELEMETRY,
        .msg_id = MSG_ID_INSTRMNT_THERMAL,
        .cmpnt_id = instruments_cmpnt->cmpnt_id,
        .cmd = 0,
    };



    /** Write the header into the Message Packet chunk */
    size_t header_bytes_written = mem_chunk_write(msg_packet_chunk, (uint8_t *)&msg_pkt_h, sizeof(Msg_Packet_H));
    if (header_bytes_written != sizeof(Msg_Packet_H)){
        printf("[INSTRUMENTS COMPONENT] <FAULT> - Thermal task: Failed publish build\n");
        thermal_channel->prod_lfs.pub_build_fault_count++;
        /** Release the requested chunk */
        swbus_release_chunk(msg_packet_chunk);
        /** Do not continue, failed to write the Message Packet header  */
        return;
    }

    /** Write the payload into the Message Packet chunk */
    size_t payload_bytes_written = mem_chunk_write(msg_packet_chunk, (uint8_t *)instruments_pub->raw_data, sizeof(Thermal_Packet));
    if (payload_bytes_written != sizeof(Thermal_Packet)){
        printf("[INSTRUMENTS COMPONENT] <FAULT> - Thermal task: Failed publish build\n");
        thermal_channel->prod_lfs.pub_build_fault_count++;
        /** Release the requested chunk */
        swbus_release_chunk(msg_packet_chunk);
        /** Do not continue, failed to write the Message Packet payload  */
        return;
    }

    /** Update Sequence number and last success frame */
    thermal_channel->sqn_number++;
    thermal_channel->prod_lfs.last_success_frame = schdlr_sys_current_frame();
    /** Instrument Manager owned memory is now safe to reuse */
    mem_chunk_clear(&instruments_pub->chunk);

    swbus_publish(msg_packet_chunk, msg_pkt_h.msg_id);


}

/**
 * 
 * @brief Entry point for processing the component queued messages. Dispatches to helpers based on MessageID
 * 
 * 
 * @param instruments_cmpnt A ```NON-NULL``` pointer to the Instruments component
 * @param msg_packet A ```MAY-BE-NULL``` pointer to a Message Packet 
 */
static void instruments_cmpnt_on_msg_received(Component *instruments_cmpnt, Msg_Packet *msg_packet)
{
    Subscriber *instruments_sub = instruments_cmpnt->subscriber;
    Channel *receive_channel = &instruments_sub->receive_channel;

    if (msg_packet == NULL){
        printf("[INSTRUMENTS COMPONENT] <FAULT> - Received NULL message packet\n");
        receive_channel->receive_lfs.receive_fault_count++;
        return;
    }

    MessageID msg_id = msg_packet->header.msg_id;
    switch(msg_id){
        case MSG_ID_HEALTH_INSTRMNT:{
            Health_Packet health_pkt;
            size_t bytes_read = msg_pkt_read_payload(msg_packet, (uint8_t *)&health_pkt, sizeof(Health_Packet));
            if (bytes_read != sizeof(Health_Packet)){
                printf("[INSTRUMENTS COMPONENT] <FAULT> - Message Packet payload reading got truncated\n");
                receive_channel->receive_lfs.receive_fault_count++;
                return;
            }
            instruments_cmpnt_handle_MSG_ID_HEALTH_INSTRMNT(instruments_cmpnt, &health_pkt);
            break;
        }

        default:{
            printf("[INSTRUMENTS COMPONENT] <FAULT> - Unknown msg id to handle\n");
            receive_channel->receive_lfs.receive_fault_count++;
            break;
        }
    }

    /** Update reception sequence number */
    receive_channel->sqn_number++;
}

/**
 * @brief Handles the Instrument component action upon receiving msg with id = ```MSG_ID_HEALTH_INSTRMNT``` 
 * 
 * @param instruments_cmpnt A ```NON-NULL``` pointer to the Instruments component
 * @param health_packet A ```NON-NULL``` pointer to the Health Packet that is the payload of the msg with id = ```MSG_ID_HEALTH_INSTRMNT```
 */
static void instruments_cmpnt_handle_MSG_ID_HEALTH_INSTRMNT(Component *instruments_cmpnt, Health_Packet *health_packet)
{
    /** Here we could perform actions based on the health of every instrument unit */
    // printf("=======================\n");
    printf("[INSTRUMENTS COMPONENT ON MSG RECEIVED] - IMU healh status: %hu\n", health_packet->imu_state);
    printf("[INSTRUMENTS COMPONENT ON MSG RECEIVED] - Thermal healh status: %hu\n", health_packet->thermal_state);
    // printf("=======================\n");

}



