#ifndef __NODESERVER_H
#define __NODESERVER_H

#include "Common.h"
#include "NodeMessage.h"
#include "GMem.h"

namespace MobNode
{

    class NodeServer{
    private:
        //ASIO
        asio::io_service* _service;
        asio::ip::udp::socket _socket;
        asio::ip::udp::endpoint _senderEndpoint;
        char _buffer[1024];
        
        //Node
        std::map<std::string, bool> _nodeMap;
        std::map<std::string, std::vector<mob::gmem<void*>>> _prgmMem;
        
    public:
        NodeServer(asio::io_service& service);
        ~NodeServer();
        
    private:
        void _startAccept();
        void _handleReceive(const boost::system::error_code& err, const size_t bytesReceived);
        void _handleSend(const boost::system::error_code& err, const size_t bytesSent);
        
        void _handleMsgPing(mob::node_message& msg);
        void _handleMsgPingLib(mob::node_message& msg);
        void _handleMsgPrgmSetMem(mob::node_message& msg);
    };

}

#endif