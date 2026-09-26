


#include "../../include/common/fault.h"


Prod_LFS init_prod_lfs()
{
    Prod_LFS prod_lfs = {
        .local_mode = LOCAL_NOMINAL,
        .last_success_frame = 0,
        .acquire_fault_count = 0,
        .pub_resource_fault_count = 0,
        .pub_build_fault_count = 0,
    };

    return prod_lfs;
}   

Receive_LFS init_receive_lfs()
{
    Receive_LFS receive_lfs = {
        .local_mode = LOCAL_NOMINAL,
        .last_success_frame = 0,
        .receive_fault_count = 0,
    };

    return receive_lfs;
}