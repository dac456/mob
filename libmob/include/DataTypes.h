#ifndef __DATATYPES_H
#define __DATATYPES_H

namespace mob
{
    
    struct float4{
        float x, y, z, w;
        
        //Constructors and utilities
        float4(){}
        float4(float _x, float _y, float _z, float _w){
            x = _x;
            y = _y;
            z = _z;
            w = _w;            
        }
        float4(std::string stringval){
            std::stringstream ss(stringval);
            
            ss >> x;
            ss >> y;
            ss >> z;
            ss >> w;
        }
        
        const std::string str() const{
            std::stringstream ss;
            ss << x << " " << y << " " << z << " " << w;
            return ss.str();
        }
        
        //Math helpers
        float length(){
            return sqrtf((x*x) + (y*y) + (z*z) + (w*w));
        }
        
        float dot(const float4& rhs){
            return (x*rhs.x) + (y*rhs.y) + (z*rhs.z) + (w*rhs.w);
        }
        
        //Operators
        inline float4 operator+(const float4& rhs) const{
            return float4(x+rhs.x, y+rhs.y, z+rhs.z, w+rhs.w);
        }
        
        inline float4 operator-(const float4& rhs) const{
            return float4(x-rhs.x, y-rhs.y, z-rhs.z, w-rhs.w);
        }
        
        inline float4 operator*(const float rhs) const{
            return float4(x*rhs, y*rhs, z*rhs, w*rhs);
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