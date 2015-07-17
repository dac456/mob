#ifndef __HOST_H    
#define __HOST_H

#include "MobCommon.h"

namespace mob
{
    
    class MOBAPI host{
    private:
        //ASIO
        asio::io_service _service;
        asio::ip::udp::socket _socket;
        asio::ip::udp::endpoint _sender_endpoint;
        char _buffer[8192];
       
        //mob
        std::string _first_host;
        std::map<std::string, bool> _node_map;
        std::map<std::pair<std::string,std::string>, bool> _kernel_status_map;
        
        bool _waiting_for_capture;
        std::vector<float> _capture_buffer;
                
    public:
        host();
        ~host();
        
        void launch(std::string prgm, std::string kernel);
        std::vector<float> capture(std::string prgm, std::string var); //TODO: overload for types?
        
        void wait(std::string prgm, std::string kernel);
        
    private:       
        void _start_accept();
        void _handle_receive(const boost::system::error_code& err, const size_t bytesReceived);     
        void _handle_send(const boost::system::error_code& err, const size_t bytesSent);   
        
        void _handle_node_ping_lib(node_message& msg);
        void _handle_get_mem(node_message& msg);
        void _handle_kernel_finished(node_message& msg);
    };

}

#endif