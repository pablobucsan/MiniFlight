

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
#include <stdio.h>
#include <stdlib.h>

void instruments_cmpnt_tick(Component *instruments_cmpnt);
void instruments_cmpnt_on_msg_received(Component *cmpnt, Msg_Packet *msg_packet);

void instruments_cmpnt_handle_health_instrmt_msg(Component *cmpnt, Health_Packet *health_packet);

void instruments_cmpnt_subify(Component *instruments_cmpnt)
{
    /** Make the subscriber */
    Subscriber *instruments_sub = make_subscriber(instruments_cmpnt);

    /** Add its subcription to IDs */
    subscriber_sub_to_msg_id(instruments_sub, MSG_ID_HEALTH_INSTRMNT);

    /** Log to the Software Bus table */
    swbus_log_subscriber(instruments_sub);

}

void init_instruments_cmpnt()
{
    Component *instruments_cmpnt = mem_sys_alloc(sizeof(Component));

    if (instruments_cmpnt == NULL){
        printf("[INSTRUMENTS COMPONENT] - Instruments component failed to allocate\n");
        exit(1);
    }

    InstrumentsManager *instruments_mng = mem_sys_alloc(sizeof(InstrumentsManager));
    if (instruments_mng == NULL){
        printf("[INSTRUMENTS COMPONENT] - Instruments manager as part of instruments component failed to allocate\n");
        exit(1);
    }

    /** Populate the manager 
     *  Initial allocated buffers that hold IMU and Thermal packets and are recycled
    */
    instruments_mng->imu_packet_buffer = mem_sys_create_buffer(sizeof(IMU_Packet));
    if (instruments_mng->imu_packet_buffer == NULL){
        printf("[INSTRUMENTS COMPONENT] - Instruments manager IMU buffer failed to allocate\n");
        exit(1);
    }
    instruments_mng->thermal_packet_buffer = mem_sys_create_buffer(sizeof(Thermal_Packet));
    if (instruments_mng->thermal_packet_buffer == NULL){
        printf("[INSTRUMENTS COMPONENT] - Instruments manager Thermal buffer failed to allocate\n");
        exit(1);
    }

    instruments_mng->imu_sqn_number = 0;
    instruments_mng->thermal_sqn_number = 0;

    /** Populate the component */
    instruments_cmpnt->cmpnt_id = CMPNT_ID_INSTRUMENTS;
    instruments_cmpnt->instruments_mng = instruments_mng;
    instruments_cmpnt->tick = instruments_cmpnt_tick;
    instruments_cmpnt->on_msg_received = instruments_cmpnt_on_msg_received;

    /** Log it to the Scheduler */
    schdlr_sys_log_cmpnt(instruments_cmpnt);

    /** Log it to the Software Bus table */
    instruments_cmpnt_subify(instruments_cmpnt);
}

void instruments_cmpnt_tick(Component *instruments_cmpnt)
{
    if (instruments_cmpnt == NULL){
        printf("[INSTRUMENTS COMPONENT] - Can't tick NULL component\n");
        return;
    }

    if (instruments_cmpnt->cmpnt_id != CMPNT_ID_INSTRUMENTS){
        printf("[INSTRUMENTS COMPONENT] - Wrong component has reached Instrument Component Tick function\n");
        exit(1);
    }

    /** Get the IMU and Thermal readings */
    IMU_Packet *imu_packet = imu_driver_read(instruments_cmpnt->instruments_mng->imu_packet_buffer);
    Thermal_Packet *thermal_packet = thermal_driver_read(instruments_cmpnt->instruments_mng->thermal_packet_buffer);

    /** Request the corresponding software bus memory buffer that is able
     * to hold both the header plus the payload
     */
    MemoryBuffer *imu_msg_packet_buffer = swbus_rqst_mem_buffer(sizeof(IMU_Packet));
    MemoryBuffer *thermal_msg_packet_buffer = swbus_rqst_mem_buffer(sizeof(Thermal_Packet));

    if (imu_msg_packet_buffer == NULL){
        printf("[INSTRUMENTS COMPONENT] - No free slot to publish imu packet\n");
        exit(1);
    }
    if (thermal_msg_packet_buffer == NULL){
        printf("[INSTRUMENTS COMPONENT] - No free slot to publish thermal packet\n");
        exit(1);
    }

    /** Create the Msg Packet */
    Msg_Packet *imu_msg_packet = (Msg_Packet *)imu_msg_packet_buffer->buffer;
    Msg_Packet *thermal_msg_packet = (Msg_Packet *)thermal_msg_packet_buffer->buffer;

    /** Populate the fields */
    imu_msg_packet->type = MSG_TYPE_TELEMETRY;
    imu_msg_packet->msg_id = MSG_ID_INSTRMNT_IMU;
    imu_msg_packet->cmpnt_id = instruments_cmpnt->cmpnt_id;
    imu_msg_packet->sqn_number = instruments_cmpnt->instruments_mng->imu_sqn_number;
    imu_msg_packet->length = sizeof(IMU_Packet);
    imu_msg_packet->cmd = 0;
    mem_sys_copy(imu_msg_packet->payload, imu_packet, sizeof(IMU_Packet));

    thermal_msg_packet->type = MSG_TYPE_TELEMETRY;
    thermal_msg_packet->msg_id = MSG_ID_INSTRMNT_THERMAL;
    thermal_msg_packet->cmpnt_id = instruments_cmpnt->cmpnt_id;
    thermal_msg_packet->sqn_number = instruments_cmpnt->instruments_mng->thermal_sqn_number;
    thermal_msg_packet->length = sizeof(IMU_Packet);
    thermal_msg_packet->cmd = 0;
    mem_sys_copy(thermal_msg_packet->payload, thermal_packet, sizeof(Thermal_Packet));


    /** Update sequence numbers */
    if (instruments_cmpnt->instruments_mng->imu_sqn_number + 1 >= MAX_SQN_NUMBER){
        instruments_cmpnt->instruments_mng->imu_sqn_number = 0;
    }
    if (instruments_cmpnt->instruments_mng->thermal_sqn_number + 1 >= MAX_SQN_NUMBER){
        instruments_cmpnt->instruments_mng->thermal_sqn_number = 0;
    }

    /** Publish to the software bus */
    swbus_publish(imu_msg_packet);
    swbus_publish(thermal_msg_packet);

    printf("[INSTRUMENTS COMPONENT] - Ticked\n");

}

void instruments_cmpnt_on_msg_received(Component *cmpnt, Msg_Packet *msg_packet)
{
    if (msg_packet == NULL){
        printf("[INSTRUMENTS COMPONENT ON MSG RECEIVED] - Received NULL message packet\n");
        return;
    }

    MessageID msg_id = msg_packet->msg_id;

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

void instruments_cmpnt_handle_health_instrmt_msg(Component *cmpnt, Health_Packet *health_packet)
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
