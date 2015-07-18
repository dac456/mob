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
        PRGM_STARTED,        //notify that the program has started running locally
        PRGM_FINISHED,       //notify that the program has ended
        PRGM_REG_KERNEL,
        HOST_EXEC_KERNEL,    //trigger kernel exec on all nodes in a given program
        HOST_GET_MEM,        //get memory contents of a named gmem object in a program
        HOST_SET_MEM,        //set memory contents of a named gmem object in a program
        KERNEL_STARTED,
        KERNEL_FINISHED,
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
        
        char _buffer[sizeof(size_t) + 1 + 8192];
        
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
        
        void decode_header(char* buffer);
        void decode_body(char* buffer);
        
        char* buffer();
        char* body_buffer();
        
        bool is_valid();
    };

}

#endif