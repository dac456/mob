#ifndef __NODECONNECTION_H
#define __NODECONNECTION_H

#include "NodeCommon.h"

namespace MobNode
{

    class NodeConnection : public boost::enable_shared_from_this<NodeConnection>{
    private:
        asio::ip::tcp::socket _socket;
        
    public:
        asio::ip::tcp::socket& getSocket();
        
        static std::shared_ptr<NodeConnection> create(asio::io_service& service);
        
    private:
        NodeConnection(asio::io_service& service);
        void _handleWrite(const boost::system::error_code& err, size_t bytesTransferred);
        
        friend class NodeServer;
    };

}

#endif