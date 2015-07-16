#ifndef __HOST_H    
#define __HOST_H

#include "MobCommon.h"

namespace mob
{
    
    class MOBAPI host{
    private:
        //ASIO
        asio::io_service _service;
        asio::ip::udp::socket _socket;
        asio::ip::udp::endpoint _sender_endpoint;
        char _buffer[512];
                
    public:
        
    };

}

#endif