#ifndef __COMMON_H
#define __COMMON_H

#include <memory>
#include <vector>
#include <utility>

#include <boost/asio.hpp>
namespace asio = boost::asio;

#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>

namespace MobNode
{
    
    typedef std::shared_ptr<asio::ip::tcp::socket> SocketPtr;
    
    class NodeConnection;
    class NodeClient;
    class NodeServer;
    
    typedef std::shared_ptr<NodeConnection> NodeConnectionPtr;
}

#endif