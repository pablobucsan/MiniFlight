

#ifndef CHANNEL_H
#define CHANNEL_H

#include "fault.h"
#include <stdint.h>


typedef enum ChannelType{
    CHANNEL_PRODUCTION,
    CHANNEL_RECEPTION
}ChannelType;

typedef struct Channel{
    uint32_t sqn_number;
    union{
        Prod_LFS prod_lfs;
        Receive_LFS receive_lfs;
    };
    ChannelType channel_type;

}Channel;

Channel channel_init(ChannelType channel_type);

#endif //CHANNEL_H
