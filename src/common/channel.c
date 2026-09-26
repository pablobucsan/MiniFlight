

#include "../../include/common/channel.h"


Channel channel_init(ChannelType channel_type)
{
    Channel c = {
        .sqn_number = 0,
        .channel_type = channel_type,
    };

    if (channel_type == CHANNEL_PRODUCTION){
        c.prod_lfs = init_prod_lfs();
    }
    else{
        c.receive_lfs = init_receive_lfs();
    }


    return c;
}