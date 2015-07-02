#ifndef __NODESERVER_H
#define __NODESERVER_H

#include "Common.h"

namespace MobNode
{

    class NodeServer{
    private:
        asio::ip::udp::socket _socket;
        asio::ip::udp::endpoint _senderEndpoint;
        char _buffer[512];
        
    public:
        NodeServer(asio::io_service& service);
        ~NodeServer();
        
    private:
        void _startAccept();
        void _handleReceive(const boost::system::error_code& err, const size_t bytesReceived);
        void _handleSend(const boost::system::error_code& err, const size_t bytesSent);
    };

}

#endif