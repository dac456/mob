#ifndef __GMALLOC_H
#define __GMALLOC_H

#include "Common.h"

namespace mob
{
    
    struct mem_size{
        int x;
        int y;
        
        mem_size(int _x, int _y=1){
            x = _x;
            y = _y;
        }
    };
    
    template<typename T> 
    MOBAPI void gmalloc(const mem_size& sz);
    
    template<typename T> 
    MOBAPI void gfree(T* mem);

}

#endif