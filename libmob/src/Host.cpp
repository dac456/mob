#include "Host.h"
#include "NodeMessage.h"

namespace mob
{
    
    host::host() : _socket(_service, asio::ip::udp::endpoint(asio::ip::udp::v4(), HOST_PORT)){
        _first_host = "";
        
        //Start an async server so we can talk to the network
        _start_accept();
        boost::thread srv(boost::bind(&asio::io_service::run, &_service));        
    }
    
    host::~host(){
        
    }
    
    void host::launch(std::string prgm, std::string kernel){
        boost::system::error_code error;
        asio::ip::udp::socket broad_socket(_service);
        broad_socket.open(asio::ip::udp::v4(), error);
        if(!error){
            broad_socket.set_option(asio::ip::udp::socket::reuse_address(true));
            broad_socket.set_option(asio::socket_base::broadcast(true));
            
            node_message msg(HOST_EXEC_KERNEL);
            
            prgm_kernel_data msg_data;
            msg_data.host_name = asio::ip::host_name();
            msg_data.prgm_name = prgm;
            msg_data.kernel_name = kernel;
            
            std::stringstream msg_stream;
            boost::archive::text_oarchive oa(msg_stream);
            oa << msg_data;            
            
            msg.set_data(msg_stream.str().c_str(), msg_stream.str().size());
            std::pair<char*, size_t> msg_pair = msg.encode();

            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), PRGM_PORT);
            broad_socket.send_to(asio::buffer(msg_pair.first, msg_pair.second), senderEndpoint);
            broad_socket.close(error);
            
            delete[] msg_pair.first;
        }
        else{
            std::cout << "broadcast error" << std::endl;
        }
        
        _kernel_status_map[std::make_pair(prgm,kernel)] = false;           
    }
    
    std::vector<float> host::capture(std::string prgm, std::string var){
        node_message msg(HOST_GET_MEM);
        _waiting_for_capture = true;
        
        prgm_var_data msg_data;
        msg_data.host_name = asio::ip::host_name();
        msg_data.prgm_name = prgm;
        msg_data.var_name = var;
        
        std::stringstream msg_stream;
        boost::archive::text_oarchive oa(msg_stream);
        oa << msg_data;              
        
        msg.set_data(msg_stream.str().c_str(), msg_stream.str().size());   
        std::pair<char*, size_t> msg_pair = msg.encode();
        
        asio::ip::udp::resolver res(_service);
        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), _first_host, boost::lexical_cast<std::string>(PRGM_PORT));
        asio::ip::udp::endpoint ep = *res.resolve(query);
        
        _socket.async_send_to(asio::buffer(msg_pair.first, msg_pair.second), ep, boost::bind(&host::_handle_send, this, asio::placeholders::error, asio::placeholders::bytes_transferred));  
        delete[] msg_pair.first;
        
       while(_waiting_for_capture);
       
       return _capture_buffer;       
    }
    
    void host::wait(std::string prgm, std::string kernel){
        while(!_kernel_status_map.at(std::make_pair(prgm,kernel)));
    }
    
    
    void host::_start_accept(){
        _socket.async_receive_from(asio::buffer(_buffer), _sender_endpoint, boost::bind(&host::_handle_receive, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
    }
    
    void host::_handle_receive(const boost::system::error_code& err, const size_t bytesReceived){
        if(err) return;
        
        node_message msg;
        msg.decode(_buffer);
        
        if(msg.is_valid()){
            switch(msg.get_type()){
                case NODE_PING_LIB:
                    _handle_node_ping_lib(msg);
                    break;
                    
                case HOST_GET_MEM:
                    _handle_get_mem(msg);
                    break;
                    
                case KERNEL_FINISHED:
                    _handle_kernel_finished(msg);
                    break;
                    
                default:
                    break;
            }
        }
            
        _start_accept();               
    }
    
    void host::_handle_send(const boost::system::error_code& err, const size_t bytesSent){
        
    }
    
    void host::_handle_node_ping_lib(node_message& msg){
        _node_map[std::string(msg.get_data())] = true;
        
        if(_first_host == ""){
            _first_host = std::string(msg.get_data());
        }
    }
    
    void host::_handle_get_mem(node_message& msg){
        std::cout << "handle_get_mem" << std::endl;
        
        //Decode message
        std::stringstream msg_stream;
        msg_stream << msg.get_data();
        
        boost::archive::text_iarchive ia(msg_stream);
        prgm_var_data data;
        ia >> data;
        
        //TODO: needs to be way more robust
        _capture_buffer.clear();
        _capture_buffer = data.var;
        _waiting_for_capture = false;        
    }
    
    void host::_handle_kernel_finished(node_message& msg){
        std::cout << "host handle kernel finished" << std::endl;
        
         //Decode message
        std::stringstream msg_stream;
        msg_stream << msg.get_data();
        
        boost::archive::text_iarchive ia(msg_stream);
        kernel_finished_data data;
        ia >> data;
        
        //Update status
        _kernel_status_map[std::make_pair(data.prgm_name, data.kernel_name)] = data.status;       
    }

}