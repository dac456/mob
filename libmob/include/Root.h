#ifndef __ROOT_H
#define __ROOT_H

#include "Common.h"

namespace mob
{

    class MOBAPI root{
    private:
        //ASIO
        asio::io_service _service;
        asio::ip::udp::socket _socket;
        asio::ip::udp::endpoint _senderEndpoint;
        char _buffer[512];
                
        //mob
        std::map<std::string, bool> _node_map;
        
    public:
        root();
        ~root();
        
        void mob_init(int argc, char* argv[]);
        void mob_kill();
        
    private:
        void _start_accept();
        void _handle_receive(const boost::system::error_code& err, const size_t bytesReceived);
        
        void _handle_node_ping_lib(node_message& msg);
    };

}

#endif