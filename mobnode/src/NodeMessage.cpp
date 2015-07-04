#include "NodeMessage.h"

namespace MobNode
{
    
    NodeMessage::NodeMessage(){
        _isValid = false;
    }
    
    NodeMessage::NodeMessage(MSG_TYPE type){
        _isValid = false;
        _header.msgType = type;
    }
    
    NodeMessage::~NodeMessage(){
        if(_body.body != nullptr){
            delete[] _body.body;
        }
    }
    
    
    void NodeMessage::setData(char* data, size_t dataLength){
        if(_body.body != nullptr){
            delete[] _body.body;
        }
        
        _body.body = new char[dataLength];
        memcpy(_body.body, data, dataLength);
        
        _header.bodySize = dataLength;
        
        //Checksum
        size_t sum = 0;
        for(size_t i=0; i<dataLength; i++){
            sum += static_cast<size_t>(_body.body[i]);
        }
        
        _body.checksum = sum;
    }
    
    char* NodeMessage::getData(){
        return _body.body;
    }
    
    std::pair<char*, size_t> NodeMessage::encode(){
        size_t bufLen = _header.bodySize + 1 + sizeof(size_t)*2;
        char* buf = new char[bufLen];
        
        buf[0] = _header.msgType;
        
        memcpy(&buf[1], reinterpret_cast<char*>(&_header.bodySize), sizeof(size_t));
        memcpy(&buf[1 + sizeof(size_t)], _body.body, _header.bodySize);
        memcpy(&buf[1 + sizeof(size_t) + _header.bodySize], reinterpret_cast<char*>(&_body.checksum), sizeof(size_t));
        
        _isValid = true; 
        return std::make_pair(buf, bufLen);
    }
    
    void NodeMessage::decode(char* buffer){
        if(buffer != nullptr){
            //Message type
            _header.msgType = buffer[0];
            size_t idx = 1;
            
            //Size of body (in bytes)
            size_t s = 0;
            for(size_t i=idx; i<sizeof(size_t) + idx; i++){
                s += static_cast<size_t>(buffer[i]) << (i - idx)*8;
            }
            _header.bodySize = s;       
            
            idx += sizeof(size_t);
            
            //Body
            size_t sum = 0;
            _body.body = new char[_header.bodySize];
            for(size_t i=idx; i<_header.bodySize + idx; i++){
                _body.body[i - idx] = buffer[i];
                sum += static_cast<size_t>(_body.body[i - idx]);
            }
            
            idx += _header.bodySize;
            
            //Body checksum
            size_t checksum = 0;
            for(size_t i=idx; i<sizeof(size_t) + idx; i++){
                checksum += static_cast<size_t>(buffer[i]) << (i - idx)*8;
            }
            
            if(checksum == sum){
                _isValid = true;
                _body.checksum = checksum;
            }
            else{
                _isValid = false;
                std::cout << "invalid checksum" << std::endl;
            }           
        }
    }
    
    bool NodeMessage::isValid(){
        return _isValid;
    }
    
}