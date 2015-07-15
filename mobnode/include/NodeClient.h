#ifndef __NODECLIENT_H
#define __NODECLIENT_H

#include "NodeCommon.h"

namespace MobNode
{

    class NodeClient{
    private:
        asio::ip::udp::socket _socket;
        char _buffer[512];
        
    public:
        NodeClient(asio::io_service& service, std::string addr);
        ~NodeClient();

    private:
        void _onSend(const boost::system::error_code& err, const size_t bytesSent);
    };

}

#endif