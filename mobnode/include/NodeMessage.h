#ifndef __NODEMESSAGE_H
#define __NODEMESSAGE_H

#include "Common.h"

namespace MobNode
{
    
    typedef enum{
        NODE_PING
    } MSG_TYPE;
    
    struct MsgHeader{
        char msgType;
        size_t bodySize;    
    };
    
    struct MsgBody{
        char* body = nullptr; //TODO: correct type?
        size_t checksum;    
    };
    
    class NodeMessage{
    private:
        MsgHeader _header;
        MsgBody _body;
        
        bool _isValid;
        
    public:
        NodeMessage();
        NodeMessage(MSG_TYPE type);
        ~NodeMessage();
        
        void setData(char* data, size_t dataLength);
        char* getData();
        
        std::pair<char*, size_t> encode();
        void decode(char* buffer);
        
        bool isValid();
    };

}

#endif