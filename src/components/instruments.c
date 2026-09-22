

#include "../../include/components/instruments.h"
#include "../../include/system/memory.h"
#include "../../include/system/scheduler.h"
#include "../../include/common/subscribers.h"
#include "../../include/common/comps.h"
#include "../../include/bus/sw_bus.h"
#include "../../include/drivers/imu_driver.h"
#include "../../include/drivers/thermal_driver.h"
#include <stdio.h>
#include <stdlib.h>

void init_instruments_cmpnt();
void instruments_cmpnt_tick(Component *instruments_cmpnt);

void instruments_cmpnt_subify(Component *instruments_cmpnt)
{
    /** Make the subscriber */
    Subscriber *instruments_sub = make_subscriber(instruments_cmpnt);

    /** Add its subcription to IDs */

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

    /** Populate the component */
    instruments_cmpnt->cmpnt_id = CMPNT_ID_INSTRUMENTS_MNG;
    instruments_cmpnt->instruments_mng = instruments_mng;
    instruments_cmpnt->init = init_instruments_cmpnt;
    instruments_cmpnt->tick = instruments_cmpnt_tick;
    instruments_cmpnt->shutdown = NULL;

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

    if (instruments_cmpnt->cmpnt_id != CMPNT_ID_INSTRUMENTS_MNG){
        printf("[INSTRUMENTS COMPONENT] - Wrong component has reached Instrument Component Tick function\n");
        exit(1);
    }

    /** Get the IMU and Thermal readings */
    IMU_Packet *imu_packet = imu_driver_read();
    Thermal_Packet *thermal_packet = thermal_driver_read();

    if (imu_packet != NULL){
        printf("[INSTRUMENTS COMPONENT] - IMU Acc Y: %.2f m/s^2\n", imu_packet->accelerometer[1]);
        printf("[INSTRUMENTS COMPONENT] - IMU Gyro Z: %.2f rad/s\n", imu_packet->gyroscope[2]);
    }
    if (thermal_packet != NULL){
        printf("[INSTRUMENTS COMPONENT] - Thermal Temp 1: %.2f C\n", thermal_packet->temp_1);
        printf("[INSTRUMENTS COMPONENT] - Thermal Temp 2: %.2f C\n", thermal_packet->temp_2);

    }

}