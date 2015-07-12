#ifndef __COMMON_H
#define __COMMON_H

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
    #define _WIN32_WINNT 0x0501
#endif

#include <memory>
#include <vector>
#include <utility>
#include <map>

#include <boost/asio.hpp>
namespace asio = boost::asio;

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
namespace bip = boost::interprocess;

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/any.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>

#include "boost/process.hpp"
namespace proc = boost::process;

#define NODE_PORT 9001

namespace MobNode
{
    
    typedef std::shared_ptr<asio::ip::tcp::socket> SocketPtr;
    
    class NodeConnection;
    class NodeClient;
    class NodeServer;
    
    typedef std::shared_ptr<NodeConnection> NodeConnectionPtr;
}

#endif