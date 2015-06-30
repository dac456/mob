#ifndef __NODECLIENT_H
#define __NODECLIENT_H

#include "Common.h"

namespace MobNode
{

    class NodeClient{
    private:
        asio::ip::tcp::socket _socket;
        char _buffer[512];
        
    public:
        NodeClient(asio::io_service& service);
        
        void onConnect(const boost::system::error_code& err, asio::ip::tcp::resolver::iterator itr);
        void onReceive(const boost::system::error_code& err);
    };

}

#endif