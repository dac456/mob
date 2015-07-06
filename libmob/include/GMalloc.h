#ifndef __GMALLOC_H
#define __GMALLOC_H

#include "Common.h"
#include "GMem.h"

namespace mob
{
    
    template<typename T> 
    MOBAPI gmem<T> gmalloc(const size_t sz){
        //gmem<T> mem = new gmem<T>(sz);
        gmem<T> mem(sz);
        return mem;
    }
    
    template<typename T> 
    MOBAPI void gfree(gmem<T> mem){
        delete mem;
    }

}

#endif