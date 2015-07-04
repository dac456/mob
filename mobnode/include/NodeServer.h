#ifndef __NODESERVER_H
#define __NODESERVER_H

#include "Common.h"

namespace MobNode
{

    class NodeServer{
    private:
        //ASIO
        asio::io_service* _service;
        asio::ip::udp::socket _socket;
        asio::ip::udp::endpoint _senderEndpoint;
        char _buffer[512];
        
        //Node
        std::map<std::string, bool> _nodeMap;
        
    public:
        NodeServer(asio::io_service& service);
        ~NodeServer();
        
    private:
        void _startAccept();
        void _handleReceive(const boost::system::error_code& err, const size_t bytesReceived);
        void _handleSend(const boost::system::error_code& err, const size_t bytesSent);
        
        void _handleMsgPing(NodeMessage& msg);
    };

}

#endif