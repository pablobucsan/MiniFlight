


#include "../../include/components/health.h"
#include "../../include/system/memory.h"
#include "../../include/system/scheduler.h"
#include "../../include/common/comps.h"
#include "../../include/common/subscribers.h"
#include "../../include/bus/sw_bus.h"
#include "../../include/drivers/imu_driver.h"
#include "../../include/drivers/thermal_driver.h"
#include "../../include/common/message.h"
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>


/** 
 * 
 * @brief File-private Health component singleton 
 *
 */
static Component health_cmpnt;

/**
 * 
 * @brief File-private Instrument publisher singleton
 */
static Health_Pub health_pub;


/**
 * Internal private helper functions
 */

static void health_cmpnt_tick(Component *health_cmpnt);
static void health_cmpnt_on_msg_received(Component *health_cmpnt, Msg_Packet *msg_packet);
static void health_cmpnt_status_task(Component *health_cmpnt);
static void health_cmpnt_msg_task(Component *health_cmpnt);

static void health_cmpnt_handle_MSG_ID_INSTRMNT_IMU(Component *health_cmpnt, IMU_Packet *imu_packet);
static void health_cmpnt_handle_MSG_ID_INSTRMNT_THERMAL(Component *health_cmpnt, Thermal_Packet *thermal_packet);

/**
 * 
 * Static assert to guarantee that Health Packet will always fit inside of 
 * Health Manager. If we add too many fields to Health Packet, compilation fails
 */

static_assert(sizeof(Health_Packet) <= MAX_TELEMETRY_PAYLOAD_SIZE,
              "Health_Packet exceeds maximum allocated telemetry buffer capacity\n");



/**
 * @brief Actions:
 * 
 * - Initializes the Health Manager, its fields and chunk
 * 
 * - Initializes the Health Component, its fields, links to the manager and to its ```tick()``` function
 * 
 * - Logs the Health Component to the Scheduler's Component table
 * 
 * - Subifies the Health Component, populating its ```.subcriber``` field
 */
void init_health_cmpnt()
{
    
    /** 
     * Set up the publisher, chunk and its production channels
     */    
    size_t data_capacity = sizeof(Health_Packet);
    health_pub.chunk = mem_chunk_init(health_pub.raw_data, sizeof(Health_Packet));
    health_pub.status_channel = channel_init(CHANNEL_PRODUCTION);

    /** Set up the subscriber, sub to IDs and log it to the Software Bus  */
    Subscriber *health_sub = make_subscriber(HEALTH_Q_SIZE, Q_DROP_OLDEST);
    subscriber_sub_to_msg_id(health_sub, MSG_ID_INSTRMNT_IMU);
    subscriber_sub_to_msg_id(health_sub, MSG_ID_INSTRMNT_THERMAL);
    swbus_log_subscriber(health_sub);

    /** Set up the component */
    health_cmpnt.cmpnt_id = CMPNT_ID_HEALTH;
    health_cmpnt.publisher = &health_pub;
    health_cmpnt.tick = health_cmpnt_tick;
    health_cmpnt.subscriber = health_sub;

    /** Log it to the Scheduler */
    schdlr_sys_log_cmpnt(&health_cmpnt);

    printf("[HEALTH COMPONENT] - Alive\n");
}

/**
 * 
 * @brief Execution of the Health component periodic tasks, like processing queued messages and assesing status
 * 
 * @warning 
 * - If a component with an id other than ```CMPNT_ID_HEALTH``` reaches this function, proceeds to ```exit()```
 * 
 * @param health_cmpnt A ```NON-NULL``` pointer to the Health component
 */
static void health_cmpnt_tick(Component *health_cmpnt)
{
    if (health_cmpnt->cmpnt_id != CMPNT_ID_HEALTH){
        printf("[HEALTH COMPONENT] - Wrong component has reached Health Component Tick function\n");
        exit(1);
    }
    printf("[HEALTH COMPONENT] - TICKING...\n");

    /** Handling incoming messages */
    health_cmpnt_msg_task(health_cmpnt);
    printf("[HEALTH COMPONENT] - Finished msg task\n");

     /**  Status task */
    health_cmpnt_status_task(health_cmpnt);
    printf("[HEALTH COMPONENT] - Finished status task\n");

    printf("[HEALTH COMPONENT] - TICKED\n");
}


/**
 * 
 * @brief Performs the message processing task of the Health component
 * 
 * @param health_cmpnt A ```NON-NULL``` pointer to the Instrument component
 */
static void health_cmpnt_msg_task(Component *health_cmpnt)
{
    /** Handling incoming messages */
    if (health_cmpnt->subscriber == NULL){
        return;
    }
    /**
     * Better to copy to the stack so if another message comes from the bus and takes that very slot,
     * we are safe
     */
    Msg_Packet msg_packet;
    while (subscriber_dequeue_msg(health_cmpnt->subscriber, &msg_packet)){
        /** On msg received is meant to, given the messages received,
         *  modify the manager owned memory (effectively writing the Health Packet)
        */
        health_cmpnt_on_msg_received(health_cmpnt, &msg_packet);
    }
    
}

/**
 * 
 * @brief Performs the status assesment task of the Health component. It publishes to the Software Bus a Memory Chunk whose payload is a Health_Packet
 * 
 * @param health_cmpnt A ```NON-NULL``` pointer to the Health component
 */
static void health_cmpnt_status_task(Component *health_cmpnt)
{
    Health_Pub *health_pub = (Health_Pub *)health_cmpnt->publisher;
    

    Channel *status_channel = &health_pub->status_channel;

    /** Request a memory chunk from the bus to form the message packet */
    size_t total_msg_size = sizeof(Msg_Packet_H) + sizeof(Health_Packet);
    MemoryChunk *msg_packet_chunk = swbus_rqst_mem_chunk(total_msg_size);


    if (msg_packet_chunk == NULL){
        printf("[HEALTH COMPONENT] <FAULT> - Status task: Failed publish resource\n");
        status_channel->prod_lfs.pub_resource_fault_count++;
        /** Do not continue, nowhere to construct the Message Packet */
        return;
    }
    
    /** Create the header for the message packet */
    Msg_Packet_H msg_pkt_h = {
        .length = sizeof(Health_Packet),
        .sqn_number = status_channel->sqn_number,
        .type = MSG_TYPE_TELEMETRY,
        .msg_id = MSG_ID_HEALTH_INSTRMNT,
        .cmpnt_id = health_cmpnt->cmpnt_id,
        .cmd = 0,
    };



    /** Write the header into the Message Packet chunk */
    size_t header_bytes_written = mem_chunk_write(msg_packet_chunk, (uint8_t *)&msg_pkt_h, sizeof(Msg_Packet_H));
    if (header_bytes_written != sizeof(Msg_Packet_H)){
        printf("[HEALTH COMPONENT] <FAULT> - Status task: Failed publish build\n");
        status_channel->prod_lfs.pub_build_fault_count++;
        /** Release the requested chunk */
        swbus_release_chunk(msg_packet_chunk);
        /** Do not continue, failed to write the Message Packet header */
        return;
    }

    /** Write the payload into the Message Packet chunk */
    size_t payload_bytes_written = mem_chunk_write(msg_packet_chunk,(uint8_t *)health_pub->raw_data, sizeof(Health_Packet));
    if (payload_bytes_written != sizeof(Health_Packet)){
        printf("[HEALTH COMPONENT] <FAULT> - Status task: Failed publish build\n");
        status_channel->prod_lfs.pub_build_fault_count++;
        /** Release the requested chunk */
        swbus_release_chunk(msg_packet_chunk);
        /** Do not continue, failed to write the Message Packet header */
        return;
    }

    /** Update Sequence number and last success frame */
    status_channel->sqn_number++;
    status_channel->prod_lfs.last_success_frame = schdlr_sys_current_frame();

    /** Health Manager owned memory is now safe to reuse */
    mem_chunk_clear(&health_pub->chunk);
    /** Software Bus given memory chunk has been populated with a header + payload */
    swbus_publish(msg_packet_chunk, msg_pkt_h.msg_id);

}

/**
 * 
 * @brief Entry point for processing the component queued messages. Dispatches to helpers based on MessageID
 * 
 * @param health_cmpnt A ```NON-NULL``` pointer to the Health component
 * @param msg_packet A ```MAY-BE-NULL``` pointer to a Message Packet 
 */
static void health_cmpnt_on_msg_received(Component *health_cmpnt, Msg_Packet *msg_packet)
{   
    Subscriber *health_sub = health_cmpnt->subscriber;
    Channel *receive_channel = &health_sub->receive_channel;

    if (msg_packet == NULL){
        printf("[HEALTH COMPONENT] <FAULT> - Received NULL message packet\n");
        receive_channel->receive_lfs.receive_fault_count++;
        return;
    }

    MessageID msg_id = msg_packet->header.msg_id;

    switch (msg_id){
        case MSG_ID_INSTRMNT_IMU:{
            
            IMU_Packet imu_packet;
            size_t bytes_read = msg_pkt_read_payload(msg_packet, (uint8_t *)&imu_packet,sizeof(IMU_Packet));
            if (bytes_read != sizeof(IMU_Packet)){
                printf("[HEALTH COMPONENT] <FAULT> - Message Packet payload reading got truncated\n");
                receive_channel->receive_lfs.receive_fault_count++;
                return;
            }
            health_cmpnt_handle_MSG_ID_INSTRMNT_IMU(health_cmpnt, &imu_packet);
            break;
        }
        case MSG_ID_INSTRMNT_THERMAL:{
            Thermal_Packet thermal_packet;
            size_t bytes_read = msg_pkt_read_payload(msg_packet, (uint8_t *)&thermal_packet, sizeof(Thermal_Packet));
            if (bytes_read != sizeof(Thermal_Packet)){
                printf("[HEALTH COMPONENT] <FAULT> - Message Packet payload reading got truncated\n");
                receive_channel->receive_lfs.receive_fault_count++;
                return;
            }
            health_cmpnt_handle_MSG_ID_INSTRMNT_THERMAL(health_cmpnt, &thermal_packet);
            break;
        }
        default:{
            printf("[HEALTH COMPONENT ON MSG RECEIVED] - Unknown msg id to handle\n");
            break;
        }
    }

    /** Update reception sequence number */
    receive_channel->sqn_number++;
}


/**
 * @brief Handles the Health component action upon receiving msg with id = ```MSG_ID_INSTRMNT_IMU``` 
 * 
 * @param health_cmpnt A ```NON-NULL``` pointer to the Health component
 * @param imu_packet A ```NON-NULL``` pointer to the IMU Packet that is the payload of the msg with id = ```MSG_ID_HEALTH_INSTRMNT```
 */
static void health_cmpnt_handle_MSG_ID_INSTRMNT_IMU(Component *health_cmpnt, IMU_Packet *imu_packet)
{

    // printf("=======================\n");
    printf("[HEALTH COMPONENT ON MSG RECEIVED] - IMU Acc Y: %.2f m/s^2\n", imu_packet->accelerometer[1]);
    printf("[HEALTH COMPONENT ON MSG RECEIVED] - IMU Gyro Z: %.2f rad/s\n", imu_packet->gyroscope[2]);
    // printf("=======================\n");


}

/**
 * @brief Handles the Health component action upon receiving msg with id = ```MSG_ID_INSTRMNT_THERMAL``` 
 * 
 * @param health_cmpnt A ```NON-NULL``` pointer to the Health component
 * @param thermal_packet A ```NON-NULL``` pointer to the Thermal Packet that is the payload of the msg with id = ```MSG_ID_INSTRMNT_THERMAL```
 */
static void health_cmpnt_handle_MSG_ID_INSTRMNT_THERMAL(Component *health_cmpnt, Thermal_Packet *thermal_packet)
{

    // printf("=======================\n");
    printf("[HEALTH COMPONENT ON MSG RECEIVED] - Thermal Temp 1: %.2f C\n", thermal_packet->temp_1);
    printf("[HEALTH COMPONENT ON MSG RECEIVED] - Thermal Temp 2: %.2f C\n", thermal_packet->temp_2);
    // printf("=======================\n");

}

