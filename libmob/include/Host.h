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
        std::vector<char> _buffer;
       
        //mob
        std::string _first_host;
        std::map<std::string, bool> _node_map;
        std::map<std::pair<std::string,std::string>, volatile bool> _kernel_status_map;
        
        //volatile bool _waiting_for_capture;
        //volatile size_t _capture_count;
        std::map<std::pair<std::string,std::string>, std::pair<size_t, volatile bool>> _capture_status_map;
        
        std::vector<float> _capture_buffer_float;
        std::vector<float4> _capture_buffer_float4;
                
    public:
        host();
        ~host();
        
        void launch(std::string prgm, std::string kernel);
        
        std::vector<float> capture_float(std::string prgm, std::string var);
        std::vector<float4> capture_float4(std::string prgm, std::string var, size_t timeout = 2000);
        
        void set_float4(std::string prgm, std::string var, std::vector<float4> val);
        
        void wait(std::string prgm, std::string kernel, size_t timeout = 5000);
        
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