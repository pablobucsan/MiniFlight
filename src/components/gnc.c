


#include "../../include/components/gnc.h"
#include "../../include/common/comps.h"
#include "../../include/common/subscribers.h"
#include "../../include/drivers/imu_driver.h"
#include "../../include/drivers/thermal_driver.h"
#include "../../include/bus/sw_bus.h"
#include "../../include/system/scheduler.h"

#include <stdlib.h>
#include <stdio.h>

/**
 * 
 * @brief File-private GNC component singleton
 */
static Component gnc_cmpnt;
/**
 * 
 * @brief File-private GNC publisher singleton
 */
static GNC_Pub gnc_pub;

/**
 * Static assert to guarantee that GNU packet will always fit inside of 
 * GNC manager
 */




/**
 * Internal private helper functions
 */

static void gnc_cmpnt_tick(Component *gnc_cmpnt);
static void gnc_cmpnt_msg_task(Component *gnc_cmpnt);
static void gnc_cmpnt_on_msg_received(Component *gnc_cmpnt, Msg_Packet *msg_packet);
static void gnc_cmpnt_handle_MSG_ID_INSTRMNT_IMU(Component *gnc_cmpnt, IMU_Packet *imu_packet);
static void gnc_cmpnt_handle_MSG_ID_INSTRMNT_THERMAL(Component *gnc_cmpnt, Thermal_Packet *thermal_packet);





/**
 * @brief Actions:
 * 
 * - Initializes the GNC Manager, its fields and chunk
 * 
 * - Initializes the GNC Component, its fields, links to the manager and to its ```tick()``` function
 * 
 * - Logs the GNC Component to the Scheduler's Component table
 * 
 * - Subifies the GNC Component, populating its ```.subcriber``` field
 */
void init_gnc_cmpnt()
{
    /** Set up the publisher, chunk and its production channels */
    size_t data_capacity = sizeof(GNC_Packet);
    gnc_pub.chunk = mem_chunk_init(gnc_pub.raw_data,data_capacity);

    /** Set up the subscriber, sub to IDs and log it to the Software Bus */
    Subscriber *gnc_sub = make_subscriber(GNC_Q_SIZE, Q_DROP_OLDEST);
    subscriber_sub_to_msg_id(gnc_sub, MSG_ID_INSTRMNT_IMU);
    subscriber_sub_to_msg_id(gnc_sub, MSG_ID_INSTRMNT_THERMAL);
    swbus_log_subscriber(gnc_sub);

    /** Set up the component */
    gnc_cmpnt.cmpnt_id = CMPNT_ID_GNC;
    gnc_cmpnt.publisher = &gnc_pub;
    gnc_cmpnt.tick = gnc_cmpnt_tick;
    gnc_cmpnt.subscriber = gnc_sub;

    /** Log it to the Scheduler */
    schdlr_sys_log_cmpnt(&gnc_cmpnt);

    printf("[GNC COMPONENT] - Alive\n");
}


/**
 * 
 * @brief Execution of the GNC components periodic tasks, like processing queued messages, performing its taks
 * 
 * @warning 
 * - If a component with an id other than ```CMPNT_ID_GNC``` reaches this function, proceeds to ```exit()```
 * 
 * @param gnc_cmpnt A ```NON-NULL``` pointer to the GNC component
 */

void gnc_cmpnt_tick(Component *gnc_cmpnt)
{
    printf("[GNC COMPONENT] - Ticking...\n");

    if (gnc_cmpnt->cmpnt_id != CMPNT_ID_GNC){
        printf("[GNC COMPONENT] - Wrong component has reached GNC component tick function\n");
        exit(1);
    }

    /** Handle incoming messages */
    gnc_cmpnt_msg_task(gnc_cmpnt);
    printf("[GNC COMPONENT] - Finished msg task\n");

    /**
     * Rest of the tick() function
     */

}

/**
 * 
 * @brief Performs the message processing task of the GNC component
 * 
 * @param gnc_cmpnt A ```NON-NULL``` pointer to the GNC component
 */

static void gnc_cmpnt_msg_task(Component *gnc_cmpnt)
{
    /** Handle incoing messages */
    if (gnc_cmpnt->subscriber == NULL){
        return;
    }

    Msg_Packet msg_packet;
    while (subscriber_dequeue_msg(gnc_cmpnt->subscriber, &msg_packet)){
        gnc_cmpnt_on_msg_received(gnc_cmpnt, &msg_packet);
    }
}

/**
 * 
 * @brief Entry point for processing the component queued messages. Dispatches to helpers based on MessageID
 * 
 * 
 * @param gnc_cmpnt A ```NON-NULL``` pointer to the GNC component
 * @param msg_packet A ```MAY-BE-NULL``` pointer to a Message Packet 
 */
static void gnc_cmpnt_on_msg_received(Component *gnc_cmpnt, Msg_Packet *msg_packet)
{
    Subscriber *gnc_sub = gnc_cmpnt->subscriber;
    Channel *receive_channel = &gnc_sub->receive_channel;

    if (msg_packet == NULL){
        printf("[GNC COMPONENT] <FAULT> - Received NULL message packet\n");
        receive_channel->receive_lfs.receive_fault_count++;
        return;
    }

    MessageID msg_id = msg_packet->header.msg_id;
    switch(msg_id){
        case MSG_ID_INSTRMNT_IMU:{
            IMU_Packet imu_packet;
            size_t bytes_read = msg_pkt_read_payload(msg_packet, (uint8_t *)&imu_packet, sizeof(IMU_Packet));
            if (bytes_read != sizeof(IMU_Packet)){
                printf("[GNC COMPONENT] <FAULT> - Message Packet payload reading got truncated\n");
                receive_channel->receive_lfs.receive_fault_count++;
                return;
            }
            gnc_cmpnt_handle_MSG_ID_INSTRMNT_IMU(gnc_cmpnt, &imu_packet);
            break;
        }

        case MSG_ID_INSTRMNT_THERMAL:{
            Thermal_Packet thermal_packet;
            size_t bytes_read = msg_pkt_read_payload(msg_packet, (uint8_t *)&thermal_packet, sizeof(Thermal_Packet));
            if (bytes_read != sizeof(Thermal_Packet)){
                printf("[GNC COMPONENT] <FAULT> - Message Packet payload reading got truncated\n");
                receive_channel->receive_lfs.receive_fault_count++;
                return;
            }
            gnc_cmpnt_handle_MSG_ID_INSTRMNT_THERMAL(gnc_cmpnt, &thermal_packet);
            break;
        }

        default:{
            printf("[GNC COMPONENT] <FAULT> - Unknown msg id to handle\n");
            receive_channel->receive_lfs.receive_fault_count++;
            break;
        }
    }

    /** Update reception sequence number */
    receive_channel->sqn_number++;
}


/**
 * @brief Handles the GNC component action upon receiving msg with id = ```MSG_ID_INSTRMNT_IMU``` 
 * 
 * @param gnc_cmpnt A ```NON-NULL``` pointer to the GNC component
 * @param imu_packet A ```NON-NULL``` pointer to the IMU Packet that is the payload of the msg with id = ```MSG_ID_INSTRMNT_IMU```
 */

static void gnc_cmpnt_handle_MSG_ID_INSTRMNT_IMU(Component *gnc_cmpnt, IMU_Packet *imu_packet)
{
    // printf("=======================\n");
    printf("[GNC COMPONENT ON MSG RECEIVED] - IMU Acc Y: %.2f m/s^2\n", imu_packet->accelerometer[1]);
    printf("[GNC COMPONENT ON MSG RECEIVED] - IMU Gyro Z: %.2f rad/s\n", imu_packet->gyroscope[2]);
    // printf("=======================\n");
}

/**
 * @brief Handles the GNC component action upon receiving msg with id = ```MSG_ID_INSTRMNT_THERMAL``` 
 * 
 * @param health_cmpnt A ```NON-NULL``` pointer to the GNC component
 * @param thermal_packet A ```NON-NULL``` pointer to the Thermal Packet that is the payload of the msg with id = ```MSG_ID_INSTRMNT_THERMAL```
 */
static void gnc_cmpnt_handle_MSG_ID_INSTRMNT_THERMAL(Component *gnc_cmpnt, Thermal_Packet *thermal_packet)
{

    // printf("=======================\n");
    printf("[GNC COMPONENT ON MSG RECEIVED] - Thermal Temp 1: %.2f C\n", thermal_packet->temp_1);
    printf("[GNC COMPONENT ON MSG RECEIVED] - Thermal Temp 2: %.2f C\n", thermal_packet->temp_2);
    // printf("=======================\n");

}