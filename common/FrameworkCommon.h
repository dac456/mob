#ifndef __FRAMEWORKCOMMON_H
#define __FRAMEWORKCOMMON_H

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
    #define _WIN32_WINNT 0x0501
#endif

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
    #define MOB_PLATFORM_WIN32
#elif defined(__GNUC__)
    #define MOB_PLATFORM_GNU
#endif

#ifdef MOB_PLATFORM_GNU


    #if defined __FAST_MATH__
    #   undef isnan
    #endif
    #if !defined isnan
    #   define isnan isnan
    #   include <stdint.h>
    static inline int isnan(float f)
    {
        union { float f; uint32_t x; } u = { f };
        return (u.x << 1) > 0xff000000u;
    }
    #endif
    

#endif

#include <memory>
#include <vector>
#include <utility>
#include <map>
#include <mutex>
#include <stack>
#include <chrono>
#include <limits>

#include <boost/asio.hpp>
namespace asio = boost::asio;

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_recursive_mutex.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/named_condition.hpp>
namespace bip = boost::interprocess;

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>

#include <boost/any.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "boost/process.hpp"
namespace proc = boost::process;

#define NODE_PORT 9001
#define PRGM_PORT 9002
#define LNCH_PORT 9003
#define HOST_PORT 9004

typedef bip::allocator<size_t, bip::managed_shared_memory::segment_manager> ShmemAllocator;
typedef std::vector<size_t, ShmemAllocator> TaskList;

static size_t total_misses = 0;

#include "DataTypes.h"

struct prgm_kernel_data{
    std::string host_name;
    std::string prgm_name;
    std::string kernel_name;
    
    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version){
        ar & host_name;
        ar & prgm_name;
        ar & kernel_name;
    }
};

struct prgm_var_data{
    std::string host_name;
    std::string prgm_name;
    std::string var_name;
    std::string var_type;
    
    //size_t start, end;
    std::vector<size_t> var_indices;
    std::vector<float> var_float;
    std::vector<mob::float4> var_float4;
    
    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version){
        ar & host_name;
        ar & prgm_name;
        ar & var_name;
        ar & var_type;
        ar & var_float;
        ar & var_float4;
        ar & var_indices;
        //ar & start;
        //ar & end;
    }    
};

struct kernel_status_data{
    std::string host_name;
    std::string prgm_name;
    std::string kernel_name;
    bool status;  
    
    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version){
        ar & host_name;
        ar & prgm_name;
        ar & kernel_name;
        ar & status;
    }          
};

struct node_task_data{
    std::string host_name;
    std::string prgm_name;
    std::vector<size_t> task_list;
    
    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version){
        ar & host_name;
        ar & prgm_name;
        ar & task_list;
    }
};   

#endif