


#include "../../include/components/health.h"
#include "../../include/system/memory.h"
#include "../../include/system/scheduler.h"
#include "../../include/common/comps.h"
#include "../../include/common/subscribers.h"
#include "../../include/bus/sw_bus.h"
#include "../../include/drivers/imu_driver.h"
#include "../../include/drivers/thermal_driver.h"
#include "../../include/common/message.h"
#include <stdio.h>
#include <stdlib.h>


/** Forward declaration of functions to reference them */
void health_cmpnt_tick(Component *health_cmpnt);
void health_cmpnt_on_msg_received(Component *health_cmpnt, Msg_Packet *msg_packet);


void health_cmpnt_handle_instrmnt_imu_msg(Component *health_cmpnt, IMU_Packet *imu_packet);
void health_cmpnt_handle_instrmnt_thermal_msg(Component *health_cmpnt, Thermal_Packet *imu_packet);


void health_cmpnt_subify(Component *health_cmpnt)
{
    /** Make the subscriber */
    Subscriber *health_sub = make_subscriber(health_cmpnt);
    /** Add its subscription to IDs */
    subscriber_sub_to_msg_id(health_sub, MSG_ID_INSTRMNT_IMU);
    subscriber_sub_to_msg_id(health_sub, MSG_ID_INSTRMNT_THERMAL);
    /** Log to the Software Bus table */
    swbus_log_subscriber(health_sub);
}

void init_health_cmpnt()
{
    Component *health_cmpnt = mem_sys_alloc(sizeof(Component));
    if (health_cmpnt == NULL){
        printf("[HEALTH COMPONENT] - Health component failed to allocate\n");
        exit(1);
    }

    HealthManager *health_mng = mem_sys_alloc(sizeof(HealthManager));
    if (health_mng == NULL){
        printf("[HEALTH COMPONENT] - Health manager as part of health component failed to allocate\n");
        exit(1);
    }

    /** Populate the component */
    health_cmpnt->cmpnt_id = CMPNT_ID_HEALTH;
    health_cmpnt->health_mng = health_mng;
    health_cmpnt->tick = health_cmpnt_tick;
    health_cmpnt->on_msg_received = health_cmpnt_on_msg_received;

    /** Populate the manager */
    health_cmpnt->health_mng->health_packet_buffer = mem_sys_create_buffer(sizeof(Health_Packet));
    health_cmpnt->health_mng->sqn_number = 0;


    /** Log it to the Scheduler */
    schdlr_sys_log_cmpnt(health_cmpnt);

    /** Log it to the Software Bus table */
    health_cmpnt_subify(health_cmpnt);

    printf("[HEALTH COMPONENT] - Alive\n");
}

void health_cmpnt_tick(Component *health_cmpnt)
{

    if (health_cmpnt == NULL){
        printf("[HEALTH COMPONENT] - Can't tick NULL component\n");
        return;
    }
    if (health_cmpnt->cmpnt_id != CMPNT_ID_HEALTH){
        printf("[HEALTH COMPONENT] - Wrong component has reached Health Component Tick function\n");
        exit(1);
    }

    /** Get the health packet */
    Health_Packet *health_packet = (Health_Packet *)health_cmpnt->health_mng->health_packet_buffer->buffer;

    /** Request the corresponding software bus memory buffer that is able
     * to hold both the header plus the payload
     */
    MemoryBuffer *health_msg_packet_buffer = swbus_rqst_mem_buffer(sizeof(Health_Packet));

    if (health_msg_packet_buffer == NULL){
        printf("[HEALTH COMPONENT] - No free slot to publish health packet\n");
        exit(1);
    }

    /** Create the message packet */
    Msg_Packet *health_msg_packet = (Msg_Packet *)health_msg_packet_buffer->buffer;

    /** Populate the fields */
    health_msg_packet->type = MSG_TYPE_TELEMETRY;
    health_msg_packet->msg_id = MSG_ID_HEALTH_INSTRMNT;
    health_msg_packet->cmpnt_id = health_cmpnt->cmpnt_id;
    health_msg_packet->sqn_number = health_cmpnt->health_mng->sqn_number;
    health_msg_packet->length = sizeof(Health_Packet);
    health_msg_packet->cmd = 0;
    mem_sys_copy(health_msg_packet->payload, health_packet, sizeof(Health_Packet));

    /** Update sequence number */
    if (health_cmpnt->health_mng->sqn_number + 1 >= MAX_SQN_NUMBER){
        health_cmpnt->health_mng->sqn_number = 0;
    }

    swbus_publish(health_msg_packet);


    printf("[HEALTH COMPONENT] - TICKED\n");
}

void health_cmpnt_on_msg_received(Component *health_cmpnt, Msg_Packet *msg_packet)
{   
    if (msg_packet == NULL){
        printf("[HEALTH COMPONENT ON MSG RECEIVED] - Received NULL message packet\n");
        return;
    }

    MessageID msg_id = msg_packet->msg_id;

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

void health_cmpnt_handle_instrmnt_imu_msg(Component *health_cmpnt, IMU_Packet *imu_packet)
{
    if (imu_packet == NULL){
        printf("[HEALTH COMPONENT] - Received NULL IMU packet\n");
        return;
    }

    Health_Packet *health_packet = (Health_Packet *)health_cmpnt->health_mng->health_packet_buffer->buffer;
    /** Here we estimate the IMU health through some calculations*/
    health_packet->imu_state = HEALTH_SAFE;

    printf("=======================\n");
    printf("[HEALTH COMPONENT ON MSG RECEIVED] - IMU Acc Y: %.2f m/s^2\n", imu_packet->accelerometer[1]);
    printf("[HEALTH COMPONENT ON MSG RECEIVED] - IMU Gyro Z: %.2f rad/s\n", imu_packet->gyroscope[2]);
    printf("[HEALTH COMPONENT ON MSG RECEIVED] - IMU State: %hu\n", health_packet->imu_state);
    printf("=======================\n");


}

void health_cmpnt_handle_instrmnt_thermal_msg(Component *health_cmpnt, Thermal_Packet *thermal_packet)
{

    if (thermal_packet == NULL){
        printf("[HEALTH COMPONENT] - Received NULL Thermal packet\n");
        return;
    }

    Health_Packet *health_packet = (Health_Packet *)health_cmpnt->health_mng->health_packet_buffer;
    /** Here we estimate the IMU health through some calculations*/
    health_packet->thermal_state = HEALTH_DEGRADED;

    printf("=======================\n");
    printf("[HEALTH COMPONENT ON MSG RECEIVED] - Thermal Temp 1: %.2f C\n", thermal_packet->temp_1);
    printf("[HEALTH COMPONENT ON MSG RECEIVED] - Thermal Temp 2: %.2f C\n", thermal_packet->temp_2);
    printf("[HEALTH COMPONENT ON MSG RECEIVED] - Thermal State: %hu\n", health_packet->thermal_state);

    printf("=======================\n");

}
