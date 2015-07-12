#ifndef __LAUNCHCOMMON_H
#define __LAUNCHCOMMON_H

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

#include <boost/any.hpp>
#include <boost/bind.hpp>
#include <boost/signals2.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>

#endif