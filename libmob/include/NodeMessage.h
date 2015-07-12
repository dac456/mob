#ifndef __NODEMESSAGE_H
#define __NODEMESSAGE_H

#include "MobCommon.h"

namespace mob
{
    
    typedef enum{
        NODE_PING,           //ping from node
        NODE_PING_LIB,       //ping from libmob
        NODE_UPDATE_TASKS,   //tasks are reassigned
        LAUNCH_PRGM,         //initial program start
        NODE_START_PRGM,     //ask node to start a program locally
        NODE_SET_TASKS,      //set task list for a program on a node
        PRGM_SET_TASKS,      //set task list in a program from a node
        PRGM_GET_MEM,        //program requests remote data
        PRGM_SET_MEM,        //program sets remote data
    } MSG_TYPE;
    
    struct msg_header{
        char msgType;
        size_t bodySize;    
    };
    
    struct msg_body{
        char* body = nullptr;
        size_t checksum;    
    };
    
    //TODO: MOBAPI causes compiler errors here
    class node_message{
    private:
        msg_header _header;
        msg_body _body;
        
        bool _is_valid;
        
    public:
        node_message();
        node_message(MSG_TYPE type);
        ~node_message();
        
        MSG_TYPE get_type();
        
        void set_data(const char* data, size_t dataLength);
        const char* get_data();
        
        msg_header get_header();
        
        std::pair<char*, size_t> encode();
        void decode(char* buffer);
        
        bool is_valid();
    };

}

#endif