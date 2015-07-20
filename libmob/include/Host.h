#ifndef __HOST_H    
#define __HOST_H

#include "MobCommon.h"
#include "NodeMessage.h"
#include "DataTypes.h"

namespace mob
{
    
    class MOBAPI host{
    private:
        //ASIO
        asio::io_service _service;
        asio::ip::udp::socket _socket;
        asio::ip::udp::endpoint _sender_endpoint;
        char _buffer[1024*1024];
       
        //mob
        std::string _first_host;
        std::map<std::string, bool> _node_map;
        std::map<std::pair<std::string,std::string>, bool> _kernel_status_map;
        
        volatile bool _waiting_for_capture;
        
        std::vector<float> _capture_buffer_float;
        std::vector<float4> _capture_buffer_float4;
                
    public:
        host();
        ~host();
        
        void launch(std::string prgm, std::string kernel);
        
        std::vector<float> capture_float(std::string prgm, std::string var);
        std::vector<float4> capture_float4(std::string prgm, std::string var);
        
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