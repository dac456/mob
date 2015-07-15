#ifndef __MOBCOMMON_H
#define __MOBCOMMON_H

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
    #define _WIN32_WINNT 0x0501
#endif

#include <functional>
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

#define NODE_PORT 9001


#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
    #define MOB_PLATFORM_WIN32
#elif defined(__GNUC__)
    #define MOB_PLATFORM_GNU
#endif

#ifdef MOBAPI_EXPORT
    #ifdef MOB_PLATFORM_WIN32
        #define MOBAPI __declspec(dllexport)
    #elif defined(MOB_PLATFORM_GNU)
        #define MOBAPI __attribute__ ((visibility ("default")))
    #endif
#else
    #ifdef MOB_PLATFORM_WIN32
        #define MOBAPI __declspec(dllimport)
    #elif defined(MOB_PLATFORM_GNU)
        #define MOBAPI
    #endif
#endif

namespace mob
{
    template<typename T> class gmem;
    class root;
    class node_message;
    class task;

}

#endif