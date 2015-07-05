#include "GMalloc.h"

namespace mob
{
    
    template<typename T> void gmalloc(mem_size sz){
        T* mem;
        
        if(sz.y > 1){
            mem = new T[sz.x][sz.y];
        }
        else{
            mem = new T[sz.x];
        }
    }
    
    template<typename T> void gfree(T* mem){
        delete[] mem;
    }

}