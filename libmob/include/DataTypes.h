#ifndef __DATATYPES_H
#define __DATATYPES_H

#include <smmintrin.h>
#include <malloc.h>

namespace mob
{
    
    struct __attribute__((aligned(16))) float4{
    //public:
        union{
            struct{
                float x, y, z, w;
            };
            __m128 val;
        };
        
        //Constructors and utilities
        float4(){
            val = _mm_setzero_ps();
        }
        float4(float _x, float _y, float _z, float _w){
            val = _mm_set_ps(_w, _z, _y, _x);
            //x = _x;
            //y = _y;
            //z = _z;
            //w = _w;           
        }
        float4(__m128 _val){
            val = _val;
        }
        float4(std::string stringval){
            std::stringstream ss(stringval);
            float _x, _y, _z, _w;
            ss >> _x;
            ss >> _y;
            ss >> _z;
            ss >> _w;
            
            val = _mm_set_ps(_w, _z, _y, _x); 
        }
        
        const std::string str() {
            std::stringstream ss;
            ss << x << " " << y << " " << z << " " << w;
            return ss.str();
        }
        
        //Math helpers
        inline float length() const{
            //return sqrtf((x*x) + (y*y) + (z*z) + (w*w));

            float D;
            _MM_EXTRACT_FLOAT(D, _mm_sqrt_ss(_mm_dp_ps(val, val, 0xFF)), 0);
            
            D;
        }
        
        inline float dot(const float4& rhs) const{
            //return (x*rhs.x) + (y*rhs.y) + (z*rhs.z) + (w*rhs.w);

            float D;
            _MM_EXTRACT_FLOAT(D, _mm_dp_ps(val, rhs.val, 0xFF), 0);
            
            return D;
        }
        
        //Operators
        inline float4 operator+(const float4& rhs) const{
            //return float4(x+rhs.x, y+rhs.y, z+rhs.z, w+rhs.w);
            return float4(_mm_add_ps(val, rhs.val));
        }
        
        inline float4 operator-(const float4& rhs) const{
            //return float4(x-rhs.x, y-rhs.y, z-rhs.z, w-rhs.w);
            return float4(_mm_sub_ps(val, rhs.val));
        }
        
        inline float4 operator*(const float rhs) const{
            //return float4(x*rhs, y*rhs, z*rhs, w*rhs);
            return float4(_mm_mul_ps(val, _mm_set1_ps(rhs)));
        }
        
        inline float4 operator/(const float rhs) const{
            //return float4(x/rhs, y/rhs, z/rhs, w/rhs);
            return float4(_mm_div_ps(val, _mm_set1_ps(rhs)));
        }
        
        inline float4 operator+(const float rhs) const{
            //return float4(x+rhs, y+rhs, z+rhs, w+rhs);
            return float4(_mm_add_ps(val, _mm_set1_ps(rhs)));
        }        
        
        template<typename Archive>
        void serialize(Archive& ar, const unsigned int version){
            ar & x;
            ar & y;
            ar & z;
            ar & w;
        }   
    };
    
}

#endif