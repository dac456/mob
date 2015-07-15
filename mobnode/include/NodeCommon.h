#ifndef __NODECOMMON_H
#define __NODECOMMON_H

#include "Common.h"

namespace MobNode
{
    
    typedef std::shared_ptr<asio::ip::tcp::socket> SocketPtr;
    
    class NodeConnection;
    class NodeClient;
    class NodeServer;
    
    typedef std::shared_ptr<NodeConnection> NodeConnectionPtr;
}

#endif