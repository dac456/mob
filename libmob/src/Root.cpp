#include "Root.h"
#include "NodeMessage.h"

namespace mob
{
    
    root::root() : _socket(_service, asio::ip::udp::endpoint(asio::ip::udp::v4(), 9001)){
        
    }
    
    root::~root(){
        
    }
    
    void root::mob_init(int argc, char* argv[]){     
        boost::thread srv(boost::bind(&asio::io_service::run, &_service));
          
        boost::system::error_code error;
        asio::ip::udp::socket broadSocket(_service);
        broadSocket.open(asio::ip::udp::v4(), error);
        if(!error){
            broadSocket.set_option(asio::ip::udp::socket::reuse_address(true));
            broadSocket.set_option(asio::socket_base::broadcast(true));
            
            node_message msg(NODE_PING_LIB);
            
            std::string node_name = asio::ip::host_name();
            msg.set_data(node_name.c_str(), node_name.length());
            
            std::pair<char*, size_t> msg_pair = msg.encode();

            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), 9001);
            broadSocket.send_to(asio::buffer(msg_pair.first, msg_pair.second), senderEndpoint);
            broadSocket.close(error);
        }
        else{
            std::cout << "broadcast error" << std::endl;
        }
    }

    void root::mob_kill(){
        //TODO: shutdown program somehow
    }
    
    
    void root::_start_accept(){
        _socket.async_receive_from(asio::buffer(_buffer), _senderEndpoint, boost::bind(&root::_handle_receive, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
    }
    
    void root::_handle_receive(const boost::system::error_code& err, const size_t bytesReceived){
        if(err) return;
        
        mob::node_message msg;
        msg.decode(_buffer);
        
        if(msg.is_valid()){
            switch(msg.get_type()){
                case mob::NODE_PING_LIB:
                    _handle_node_ping_lib(msg);
                    break;
                    
                default:
                    break;
            }
        }
            
        _start_accept();        
    }
    
    void root::_handle_node_ping_lib(node_message& msg){
        _node_map[std::string(msg.get_data())] = true;
    }
    
}