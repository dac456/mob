#ifndef __DATATYPES_H
#define __DATATYPES_H

namespace mob
{
    
    struct float4{
        float x, y, z, w;
        
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
        
        std::string str(){
            std::stringstream ss;
            ss << x << " " << y << " " << z << " " << w;
            return ss.str();
        }
        
        inline float4 operator+(const float4& rhs) const{
            return float4(x+rhs.x, y+rhs.y, z+rhs.z, w+rhs.w);
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