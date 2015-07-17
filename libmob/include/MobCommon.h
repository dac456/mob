#ifndef __MOBCOMMON_H
#define __MOBCOMMON_H

#include "Common.h"


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
    class kernel;
    class host;

}

#endif