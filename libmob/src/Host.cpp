#include "Host.h"

namespace mob
{
    
    host::host() : _socket(_service, asio::ip::udp::endpoint(asio::ip::udp::v4(), HOST_PORT)){
        _first_host = "";
        //_capture_count = 0;
        _buffer.resize((1024*1024)*16);
        
        //Start an async server so we can talk to the network
        _socket.set_option(asio::ip::udp::socket::reuse_address(true));
        _socket.set_option(asio::socket_base::broadcast(true));        
        
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
    
    std::vector<float> host::capture_float(std::string prgm, std::string var){
        node_message msg(HOST_GET_MEM);
        //_waiting_for_capture = true;
        for(auto node : _node_map){
            _capture_status_map[std::make_pair(node.first, var)] = std::make_pair(0, false);     
        }        
        
        prgm_var_data msg_data;
        msg_data.host_name = asio::ip::host_name();
        msg_data.prgm_name = prgm;
        msg_data.var_name = var;
        msg_data.var_type = "float";
        
        std::stringstream msg_stream;
        boost::archive::text_oarchive oa(msg_stream);
        oa << msg_data;              
        
        msg.set_data(msg_stream.str().c_str(), msg_stream.str().size());   
        std::pair<char*, size_t> msg_pair = msg.encode();
        
        //asio::ip::udp::resolver res(_service);
        //asio::ip::udp::resolver::query query(asio::ip::udp::v4(), _first_host, boost::lexical_cast<std::string>(PRGM_PORT));
        //asio::ip::udp::endpoint ep = *res.resolve(query);
        asio::ip::udp::endpoint ep(asio::ip::address_v4::broadcast(), PRGM_PORT);
        
        _socket.async_send_to(asio::buffer(msg_pair.first, msg_pair.second), ep, boost::bind(&host::_handle_send, this, asio::placeholders::error, asio::placeholders::bytes_transferred));  
        delete[] msg_pair.first;
        
       //while(_waiting_for_capture);
       for(auto node: _node_map){
           while(!_capture_status_map[std::make_pair(node.first, var)].second);
       }
       
       return _capture_buffer_float;
    } 
    
    std::vector<float4> host::capture_float4(std::string prgm, std::string var, size_t timeout){
        node_message msg(HOST_GET_MEM);
        //_waiting_for_capture = true;
        for(auto node : _node_map){
            _capture_status_map[std::make_pair(node.first, var)] = std::make_pair(0, false);     
        }           
        
        prgm_var_data msg_data;
        msg_data.host_name = asio::ip::host_name();
        msg_data.prgm_name = prgm;
        msg_data.var_name = var;
        msg_data.var_type = "float4";
        
        std::stringstream msg_stream;
        boost::archive::text_oarchive oa(msg_stream);
        oa << msg_data;              
        
        msg.set_data(msg_stream.str().c_str(), msg_stream.str().size());   
        std::pair<char*, size_t> msg_pair = msg.encode();
        
        //asio::ip::udp::resolver res(_service);
        //asio::ip::udp::resolver::query query(asio::ip::udp::v4(), _first_host, boost::lexical_cast<std::string>(PRGM_PORT));
        //asio::ip::udp::endpoint ep = *res.resolve(query);
        asio::ip::udp::endpoint ep(asio::ip::address_v4::broadcast(), PRGM_PORT);
        
        _socket.async_send_to(asio::buffer(msg_pair.first, msg_pair.second), ep, boost::bind(&host::_handle_send, this, asio::placeholders::error, asio::placeholders::bytes_transferred));  
        delete[] msg_pair.first;
        
       //while(_waiting_for_capture);
       for(auto node: _node_map){
           std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
           std::cout << node.first << std::endl;
           while(!_capture_status_map[std::make_pair(node.first, var)].second){
                if(timeout != -1){
                    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
                    if(millis > timeout){
                        break;
                    }
                }
           }
       }       

       return _capture_buffer_float4;
    }     
    
    void host::set_float4(std::string prgm, std::string var, std::vector<float4> val){
        size_t block_size = floor(200.0f/16.0f); //TODO: don't hardcode task list
        for(size_t i=0; i<16; i++){
            size_t start = i*block_size;
            size_t end = start+block_size;
                
            node_message msg(HOST_SET_MEM); 
            
            prgm_var_data msg_data;
            msg_data.host_name = asio::ip::host_name();
            msg_data.prgm_name = prgm;
            msg_data.var_name = var;
            msg_data.var_type = "float4";
            
            for(size_t j=start; j<end; j++){
                msg_data.var_float4.push_back(val[j]);
                msg_data.var_indices.push_back(j);
            }        
            
            std::stringstream msg_stream;
            boost::archive::text_oarchive oa(msg_stream);
            oa << msg_data;              
            
            msg.set_data(msg_stream.str().c_str(), msg_stream.str().size());   
            std::pair<char*, size_t> msg_pair = msg.encode();
            asio::ip::udp::endpoint ep(asio::ip::address_v4::broadcast(), PRGM_PORT);
            
            _socket.send_to(asio::buffer(msg_pair.first, msg_pair.second), ep);  
            delete[] msg_pair.first;        
        }
    }
    
    void host::wait(std::string prgm, std::string kernel, size_t timeout){
        std::cout << "host waiting..." << std::endl;
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        while(!_kernel_status_map.at(std::make_pair(prgm,kernel))){
            if(timeout != -1){
                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
                if(millis > timeout){
                    break;
                }   
            }        
        }
        std::cout << "kernel finished" << std::endl;
    }
    
    
    void host::_start_accept(){
        _socket.async_receive_from(asio::buffer(_buffer), _sender_endpoint, boost::bind(&host::_handle_receive, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
    }
    
    void host::_handle_receive(const boost::system::error_code& err, const size_t bytesReceived){
        if(err) return;
        
        node_message msg;
        msg.decode(&_buffer[0]);
        
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
        std::cout << "_handle_get_mem" << std::endl;
        //Decode message
        std::stringstream msg_stream;
        msg_stream << msg.get_data();
        
        boost::archive::text_iarchive ia(msg_stream);
        prgm_var_data data;
        ia >> data;
        std::cout << data.host_name << std::endl;
        
        //TODO: needs to be way more robust
        if(data.var_type == "float"){
            if(data.var_indices.size()){
                size_t max = (*std::max_element(data.var_indices.begin(), data.var_indices.end())) + 1;
                if(max > _capture_buffer_float.size()){
                    _capture_buffer_float.resize(max);
                }  
                //std::copy(data.var_float.begin(), data.var_float.end(), _capture_buffer_float.begin() + data.start);
                size_t i=0;
                for(auto v : data.var_float){
                    _capture_buffer_float[data.var_indices[i]] = v;
                    i++;
                }
            }
        }
        else if(data.var_type == "float4"){
            if(data.var_indices.size()){
                size_t max = (*std::max_element(data.var_indices.begin(), data.var_indices.end())) + 1;
                if(max > _capture_buffer_float4.size()){
                    _capture_buffer_float4.resize(max);
                }   
    
                //std::copy(data.var_float4.begin(), data.var_float4.end(), _capture_buffer_float4.begin() + data.start);     
                size_t i=0;
                for(auto v : data.var_float4){
                    _capture_buffer_float4[data.var_indices[i]] = v;
                    i++;
                }   
            }         
        }
        
        /*if(_capture_count >= 3){
            _waiting_for_capture = false;  
        }
        _capture_count = (_capture_count + 1) % 4;*/
        _capture_status_map[std::make_pair(data.host_name, data.var_name)].first++;
        if(_capture_status_map[std::make_pair(data.host_name, data.var_name)].first >= 16){
            _capture_status_map[std::make_pair(data.host_name, data.var_name)].first = 0;
            _capture_status_map[std::make_pair(data.host_name, data.var_name)].second = true;
        }
    }
    
    void host::_handle_kernel_finished(node_message& msg){
        std::cout << "host handle kernel finished" << std::endl;
        
         //Decode message
        std::stringstream msg_stream;
        msg_stream << msg.get_data();
        
        boost::archive::text_iarchive ia(msg_stream);
        kernel_status_data data;
        ia >> data;
        
        //Update status
        _kernel_status_map[std::make_pair(data.prgm_name, data.kernel_name)] = data.status;       
    }

}