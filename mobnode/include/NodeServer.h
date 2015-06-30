#ifndef __NODESERVER_H
#define __NODESERVER_H

#include "Common.h"

namespace MobNode
{

    class NodeServer{
    private:
        asio::ip::tcp::acceptor _acc;
        
    public:
        NodeServer(asio::io_service& service);
        ~NodeServer();
        
    private:
        void _startAccept();
        void _handleAccept(NodeConnectionPtr session, const boost::system::error_code& err);
    };

}

#endif