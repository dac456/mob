#ifndef __NODECONNECTION_H
#define __NODECONNECTION_H

#include "Common.h"

namespace MobNode
{

    class NodeConnection : public boost::enable_shared_from_this<NodeConnection>{
    private:
        asio::ip::tcp::socket _socket;
        
    public:
        NodeConnection(asio::io_service& service);
        asio::ip::tcp::socket& getSocket();
        
    private:
        void _handleWrite(const boost::system::error_code& err, size_t bytesTransferred);
        
        friend class NodeServer;
    };

}

#endif