
#ifndef FAULT_H
#define FAULT_H

#include <stdint.h>

typedef enum FaultType{
    FAULT_ACQUIRE,
    FAULT_PUB_RESOURCE,   /** publish resource */
    FAULT_PUB_BUILD,      /** publish build */
    FAULT_RECEIVE
}FaultType;

typedef enum AcquireState{
    ACQUIRE_GOOD,
    ACQUIRE_DEGRADED,
    ACQUIRE_BAD
}AcquireState;


typedef enum LocalMode{
    LOCAL_NOMINAL,
    LOCAL_DEGRADED,
}LocalMode;

typedef struct Prod_LFS{
    LocalMode local_mode;
    uint32_t last_success_frame;
    uint8_t acquire_fault_count;
    uint8_t pub_resource_fault_count;
    uint8_t pub_build_fault_count;
}Prod_LFS;

typedef struct Receive_LFS{
    LocalMode local_mode;
    uint32_t last_success_frame;
    uint8_t receive_fault_count;
}Receive_LFS;

Prod_LFS init_prod_lfs();
Receive_LFS init_receive_lfs();


#endif //FAULT_H