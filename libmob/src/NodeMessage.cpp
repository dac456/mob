#include "NodeMessage.h"

namespace mob
{
    
    node_message::node_message(){
        _is_valid = false;
        //_buffer = new char[sizeof(size_t) + 1 + ((1024*1024)*2)];
    }
    
    node_message::node_message(MSG_TYPE type){
        _is_valid = false;
        _header.msgType = type;
        //_buffer = new char[sizeof(size_t) + 1 + ((1024*1024)*2)];
    }
    
    node_message::~node_message(){
        if(_body.body != nullptr){
            delete[] _body.body;
            _body.body = nullptr;
        }
        
        //delete _buffer;
    }
    
    MSG_TYPE node_message::get_type(){
        return static_cast<MSG_TYPE>(_header.msgType);
    }
    
    void node_message::set_data(const char* data, size_t dataLength){
        if(_body.body != nullptr){
            delete[] _body.body;
        }
        
        _body.body = new char[dataLength + 1];
        memcpy(_body.body, data, dataLength);
        _body.body[dataLength] = '\0';
        
        _header.bodySize = dataLength + 1;
        
        //Checksum
        size_t sum = 0;
        for(size_t i=0; i<dataLength + 1; i++){
            sum += static_cast<size_t>(_body.body[i]);
        }
        
        _body.checksum = sum;
    }
    
    const char* node_message::get_data(){
        return _body.body;
    }
    
    msg_header node_message::get_header(){
        return _header;
    }
    
    std::pair<char*, size_t> node_message::encode(){
        size_t bufLen = _header.bodySize + 11 + sizeof(size_t);
        char* buf = new char[bufLen];
        //char buf[bufLen];
        
        buf[0] = _header.msgType;
        
        //memcpy(&buf[1], reinterpret_cast<char*>(&_header.bodySize), sizeof(size_t));
        char header[11] = "";
        sprintf(header, "%10d", (int)_header.bodySize);
        memcpy(&buf[1], header, 10);
        memcpy(&buf[1 + 10], _body.body, _header.bodySize);
        memcpy(&buf[1 + 10 + _header.bodySize], reinterpret_cast<char*>(&_body.checksum), sizeof(size_t));
        
        _is_valid = true; 
        return std::make_pair(buf, bufLen);
    }
    
    std::pair<std::vector<char>, size_t> node_message::encode_vec(){
        size_t bufLen = _header.bodySize + 1 + sizeof(size_t)*2;
        //char* buf = new char[bufLen];
        //char buf[bufLen];
        std::vector<char> buf;
        buf.resize(bufLen);
        
        buf[0] = _header.msgType;
        
        //memcpy(&buf[1], reinterpret_cast<char*>(&_header.bodySize), sizeof(size_t));
        char header[11] = "";
        sprintf(header, "%10d", (int)_header.bodySize);
        memcpy(&buf[1], header, 10);
        memcpy(&buf[1 + 10], _body.body, _header.bodySize);
        memcpy(&buf[1 + 10 + _header.bodySize], reinterpret_cast<char*>(&_body.checksum), sizeof(size_t));
        
        _is_valid = true; 
        return std::make_pair(buf, bufLen);
    }    
    
    void node_message::decode(char* buffer){
        if(buffer != nullptr){
            //Message type
            _header.msgType = buffer[0];
            size_t idx = 1;
            
            //Size of body (in bytes)
            /*size_t s = 0;
            for(size_t i=idx; i<sizeof(size_t) + idx; i++){
                s += static_cast<size_t>(buffer[i]) << (i - idx)*8;
            }
            _header.bodySize = s;       
            
            idx += sizeof(size_t);*/
            char header[11] = "";
            strncat(header, buffer + 1, 10);
            _header.bodySize = atoi(header);
            idx += 10;
            
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
            
            //if(checksum == sum){
                _is_valid = true;
                _body.checksum = checksum;
            //}
            //else{
            //    _is_valid = false;
            //    std::cout << "invalid checksum" << std::endl;
            //}           
        }
    }
    
    void node_message::decode_header(char* buffer){
        if(buffer != nullptr){
            //Message type
            _header.msgType = buffer[0];
            size_t idx = 1;
            
            //Size of body (in bytes)
            /*size_t s = 0;
            for(size_t i=idx; i<sizeof(size_t) + idx; i++){
                s += static_cast<size_t>(buffer[i]) << ((i - idx)*8);
            }
            _header.bodySize = s;*/
            char header[11] = "";
            strncat(header, buffer + 1, 10);
            _header.bodySize = atoi(header);
            //idx += 4;            
        }        
    }
    
    void node_message::decode_body(char* buffer){
        if(buffer != nullptr){
            //size_t idx = 1 + sizeof(size_t);
            size_t idx = 0;
            
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
            
            //if(checksum == sum){
                _is_valid = true;
                _body.checksum = checksum;
            //}
            //else{
            //    _is_valid = false;
            //    std::cout << "invalid checksum" << std::endl;
            //} 
        }         
    }
    
    char* node_message::buffer(){
        return _buffer;
    }
    
    char* node_message::body_buffer(){
        return _buffer + (10 + 1);
    }
    
    bool node_message::is_valid(){
        return _is_valid;
    }
    
}