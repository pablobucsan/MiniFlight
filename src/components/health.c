


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
 * File-private static singletons 
 *
 */
static Component health_cmpnt;
static HealthManager health_mng;


/**
 * Internal private helper functions
 */
static void health_cmpnt_tick(Component *health_cmpnt);
static void health_cmpnt_on_msg_received(Component *health_cmpnt, Msg_Packet *msg_packet);
static void health_cmpnt_handle_instrmnt_imu_msg(Component *health_cmpnt, IMU_Packet *imu_packet);
static void health_cmpnt_handle_instrmnt_thermal_msg(Component *health_cmpnt, Thermal_Packet *imu_packet);
static void health_cmpnt_subify(Component *health_cmpnt);
static void health_cmpnt_status_task(Component *health_cmpnt);
static void health_cmpnt_msg_task(Component *health_cmpnt);

/**
 * For other components to be able to deserialize the data
 */

/**
 * 
 * Static assert to guarantee that Health Packet will always fit inside of 
 * Health Manager. If we add too many fields to Health Packet, compilation fails
 */

static_assert(sizeof(Health_Packet) <= MAX_TELEMETRY_PAYLOAD_SIZE,
              "Health_Packet exceeds maximum allocated telemetry buffer capacity\n");



static void health_cmpnt_subify(Component *health_cmpnt)
{
    /** Make the subscriber and link to the component*/
    Subscriber *health_sub = make_subscriber(HEALTH_Q_SIZE, Q_DROP_OLDEST);
    health_cmpnt->subscriber = health_sub;
    /** Add its subscription to IDs */
    subscriber_sub_to_msg_id(health_sub, MSG_ID_INSTRMNT_IMU);
    subscriber_sub_to_msg_id(health_sub, MSG_ID_INSTRMNT_THERMAL);
    /** Log to the Software Bus table */
    swbus_log_subscriber(health_sub);
}

void init_health_cmpnt()
{
    
    /** Populate the manager */
    health_mng.sqn_number = 0;
    health_mng.chunk = mem_chunk_init(health_mng.raw_data, sizeof(Health_Packet));
    
    /** Populate the component */
    health_cmpnt.cmpnt_id = CMPNT_ID_HEALTH;
    health_cmpnt.mng = &health_mng;
    health_cmpnt.tick = health_cmpnt_tick;
    health_cmpnt.on_msg_received = health_cmpnt_on_msg_received;

    /** Log it to the Scheduler */
    schdlr_sys_log_cmpnt(&health_cmpnt);

    /** Log it to the Software Bus table */
    health_cmpnt_subify(&health_cmpnt);

    printf("[HEALTH COMPONENT] - Alive\n");
}

/**
 * 
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
    /**
     *  Rest of the tick() function. Construct the Msg Packet with 
     *  the payload being the manager owned memory that was written into 
     *  by function helpers
     */

     /**  Status task */
    health_cmpnt_status_task(health_cmpnt);

    printf("[HEALTH COMPONENT] - TICKED\n");
}

static void health_cmpnt_msg_task(Component *health_cmpnt)
{
    /** Handling incoming messages */
    if (health_cmpnt->subscriber == NULL){
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
    while (subscriber_dequeue_msg(health_cmpnt->subscriber, &msg_packet)){
        /** On msg received is meant to, given the messages received,
         *  modify the manager owned memory (effectively writing the Health Packet)
        */
        health_cmpnt_on_msg_received(health_cmpnt, &msg_packet);
    }
    
}

static void health_cmpnt_status_task(Component *health_cmpnt)
{
 HealthManager *health_mng = (HealthManager *)health_cmpnt->mng;
    

    /** Request a memory chunk from the bus to form the message packet */
    size_t total_msg_size = sizeof(Msg_Packet_H) + sizeof(Health_Packet);
    MemoryChunk *msg_packet_chunk = swbus_rqst_mem_chunk(total_msg_size);

    if (msg_packet_chunk == NULL){
        printf("[HEALTH COMPONENT] - No memory chunk available to publish msg\n");
        return;
    }
    
    /** Create the header for the message packet */
    Msg_Packet_H msg_pkt_h = {
        .length = sizeof(Health_Packet),
        .sqn_number = health_mng->sqn_number,
        .type = MSG_TYPE_TELEMETRY,
        .msg_id = MSG_ID_HEALTH_INSTRMNT,
        .cmpnt_id = health_cmpnt->cmpnt_id,
        .cmd = 0,
    };

    /** Update sequence number */
    health_mng->sqn_number++;
    if (health_mng->sqn_number >= MAX_SQN_NUMBER){
        health_mng->sqn_number = 0;
    }

    /** Write the header into the Message Packet chunk */
    size_t header_bytes_written = mem_chunk_write(msg_packet_chunk, (uint8_t *)&msg_pkt_h, sizeof(Msg_Packet_H));
    if (header_bytes_written != sizeof(Msg_Packet_H)){
        printf("[HEALTH COMPONENT] - Header writing to msg packet got truncated, written: %zu, meant to write: %zu\n",
                        header_bytes_written, sizeof(Msg_Packet_H));
        exit(1);
    }

    /** Write the payload into the Message Packet chunk */
    size_t payload_bytes_written = mem_chunk_write(msg_packet_chunk,(uint8_t *)health_mng->raw_data, sizeof(Health_Packet));
    if (payload_bytes_written != sizeof(Health_Packet)){
        printf("[HEALTH COMPONENT] - Payload writing to msg packet got truncated\n");
        exit(1);
    }


    /** Health Manager owned memory is now safe to reuse */
    mem_chunk_clear(&health_mng->chunk);

    /** Software Bus given memory chunk has been populated with a header + payload */
    swbus_publish(msg_packet_chunk, msg_pkt_h.msg_id);
}

static void health_cmpnt_on_msg_received(Component *health_cmpnt, Msg_Packet *msg_packet)
{   
    if (msg_packet == NULL){
        printf("[HEALTH COMPONENT ON MSG RECEIVED] - Received NULL message packet\n");
        return;
    }

    MessageID msg_id = msg_packet->header.msg_id;

    switch (msg_id){
        case MSG_ID_INSTRMNT_IMU:{
            IMU_Packet *imu_packet = (IMU_Packet *)msg_packet->payload;
            health_cmpnt_handle_instrmnt_imu_msg(health_cmpnt, imu_packet);
            break;
        }
        case MSG_ID_INSTRMNT_THERMAL:{
            Thermal_Packet *thermal_packet = (Thermal_Packet *)msg_packet->payload;
            health_cmpnt_handle_instrmnt_thermal_msg(health_cmpnt, thermal_packet);
            break;
        }
        default:{
            printf("[HEALTH COMPONENT ON MSG RECEIVED] - Unknown msg id to handle\n");
            break;
        }
    }
}

static void health_cmpnt_handle_instrmnt_imu_msg(Component *health_cmpnt, IMU_Packet *imu_packet)
{
    if (imu_packet == NULL){
        printf("[HEALTH COMPONENT] - Received NULL IMU packet\n");
        return;
    }


    printf("=======================\n");
    printf("[HEALTH COMPONENT ON MSG RECEIVED] - IMU Acc Y: %.2f m/s^2\n", imu_packet->accelerometer[1]);
    printf("[HEALTH COMPONENT ON MSG RECEIVED] - IMU Gyro Z: %.2f rad/s\n", imu_packet->gyroscope[2]);
    printf("=======================\n");


}

static void health_cmpnt_handle_instrmnt_thermal_msg(Component *health_cmpnt, Thermal_Packet *thermal_packet)
{

    if (thermal_packet == NULL){
        printf("[HEALTH COMPONENT] - Received NULL Thermal packet\n");
        return;
    }


    printf("=======================\n");
    printf("[HEALTH COMPONENT ON MSG RECEIVED] - Thermal Temp 1: %.2f C\n", thermal_packet->temp_1);
    printf("[HEALTH COMPONENT ON MSG RECEIVED] - Thermal Temp 2: %.2f C\n", thermal_packet->temp_2);

    printf("=======================\n");

}

