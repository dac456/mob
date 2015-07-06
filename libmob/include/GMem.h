#ifndef __GMEM_H
#define __GMEM_H

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
    class MOBAPI gmem{
    private:
        T* _data;
        const size_t _sz;
        
    public:
        gmem(const size_t sz) : _sz(sz){
            _data = new T[sz];
        }
        
        ~gmem(){
            delete[] _data;
        }
        
        void set(size_t idx, T val){
            _data[idx] = val;
        }
        
        size_t size(){
            return _sz;
        }
        
        const T& operator[] (const int idx) const{
            assert(idx >= 0 && idx < _sz);
            return _data[idx];
        }
        
    };
    
}

#endif