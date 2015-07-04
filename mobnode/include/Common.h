#ifndef __COMMON_H
#define __COMMON_H

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
    #define _WIN32_WINNT 0x0501
#endif

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
    class NodeMessage;
    
    typedef std::shared_ptr<NodeConnection> NodeConnectionPtr;
}

#endif