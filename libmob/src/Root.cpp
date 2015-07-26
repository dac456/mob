#include "Root.h"
#include "NodeMessage.h"
#include "GMem.h"
#include "Kernel.h"

namespace mob
{
    
    //TODO: program ports should be incremented to allow multiple programs per node
    root::root() : _socket(_service, asio::ip::udp::endpoint(asio::ip::udp::v4(), PRGM_PORT)){
        _waiting_for_tasks = true;
        _buffer.resize((1024*1024)*16);
    }
    
    root::~root(){

    }
    
    void root::init(int argc, char* argv[]){
        _prgm_name = fs::path(argv[0]).filename().string();
        std::cout << _prgm_name << std::endl;
        
        int nextParam = 0;
        for(int i=0; i<argc; i++){
            if(strstr(argv[i], "--") == nullptr){
                if(nextParam == 1){
                    //_task_indices.push_back(atoi(argv[i]));
                }
            }
            else{     
                if(strcmp(argv[i], "--tasklist") == 0){
                    nextParam = 1;
                }
            }            
        }
        
        //Did we get tasks?
        /*if(!_task_indices.empty()){
            _waiting_for_tasks = false;
        }*/
        
        //Create shared memory pool
        bip::named_condition::remove("test_cnd");
        bip::named_mutex::remove("test_mtx");
        
        bip::shared_memory_object::remove(_prgm_name.c_str()); //Clear anything left from previous execution
        bip::shared_memory_object::remove("task_list");
        
        try{
            std::cout << "creating memory" << std::endl;
            bip::managed_shared_memory segment(bip::open_or_create, _prgm_name.c_str(), (1024*1024)*32);
            segment.construct<TaskList>("task_list")(segment.get_segment_manager());
            
            //bip::named_condition cond(bip::create_only,  "test_cnd");
            //bip::named_mutex task_mutex(bip::create_only, "test_mtx");
        }
        catch(...){
            std::cout << "exception" << std::endl;
            bip::shared_memory_object::remove(_prgm_name.c_str());
        }
        
        //Broadcast that the program has started on the local node
        boost::system::error_code error;
        asio::ip::udp::socket broad_socket(_service);
        broad_socket.open(asio::ip::udp::v4(), error);
        if(!error){
            broad_socket.set_option(asio::ip::udp::socket::reuse_address(true));
            broad_socket.set_option(asio::socket_base::broadcast(true));
            
            node_message msg(PRGM_STARTED);
            
            prgm_started_data data;
            data.node_name = asio::ip::host_name();
            data.prgm_name = _prgm_name;
            data.status = true;
            
            std::stringstream msg_stream;
            boost::archive::text_oarchive oa(msg_stream);
            oa << data;
                        
            msg.set_data(msg_stream.str().c_str(), msg_stream.str().size());
            std::pair<char*, size_t> msg_pair = msg.encode();
            
            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), NODE_PORT);
            broad_socket.send_to(asio::buffer(msg_pair.first, msg_pair.second), senderEndpoint);
            broad_socket.close(error);
        }
        else{
            std::cout << "broadcast error" << std::endl;
        }      
        
        //Start an async server so we can talk to the network
        _socket.set_option(asio::ip::udp::socket::reuse_address(true));
        _socket.set_option(asio::socket_base::broadcast(true));        
        
        _start_accept();
        for(size_t i=0; i<10; i++){
            boost::thread srv(boost::bind(&asio::io_service::run, &_service));
        }
    }

    void root::kill(){
        _socket.close();
        bip::shared_memory_object::remove(_prgm_name.c_str());
    }
    
    void root::run(){
        for(;;){
            //TODO: poll for kill signal
        }
    }
    
    void root::add_kernel(kernel& kern){
        _kernel_map[kern.get_name()] = &kern;
        
        //Broadcast kernel name to nodes
        boost::system::error_code error;
        asio::ip::udp::socket broad_socket(_service);
        broad_socket.open(asio::ip::udp::v4(), error);
        if(!error){
            broad_socket.set_option(asio::ip::udp::socket::reuse_address(true));
            broad_socket.set_option(asio::socket_base::broadcast(true));
            
            node_message msg(PRGM_REG_KERNEL);
            
            prgm_kernel_data data;
            data.host_name = asio::ip::host_name();
            data.prgm_name = _prgm_name;
            data.kernel_name = kern.get_name();
            
            std::stringstream msg_stream;
            boost::archive::text_oarchive oa(msg_stream);
            oa << data;            
            
            msg.set_data(msg_stream.str().c_str(), msg_stream.str().size());
            std::pair<char*, size_t> msg_pair = msg.encode();

            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), NODE_PORT);
            broad_socket.send_to(asio::buffer(msg_pair.first, msg_pair.second), senderEndpoint);
            broad_socket.close(error);
            
            delete[] msg_pair.first;
        }
        else{
            std::cout << "broadcast error" << std::endl;
        }         
    }
    
    void root::exec_kernel(std::string name){
        _kernel_map.at(name)->_exec(*this);
    }
    
    const std::string root::get_name() const{
        return _prgm_name;
    }
    
    bool root::is_waiting_for_tasks(){
        return _waiting_for_tasks;
    }
    
    TaskList* root::get_task_list(){
        //bip::scoped_lock<bip::named_mutex> lock(task_mutex);
        
        std::cout << "hang 1" << std::endl;
        bip::managed_shared_memory segment(bip::open_only, _prgm_name.c_str());      
        std::cout << "hang 2" << std::endl;
        TaskList* ret = segment.find<TaskList>("task_list").first;
        
        return ret;
    }
    
    
    void root::_start_accept(){
        _socket.async_receive_from(asio::buffer(_buffer), _sender_endpoint, boost::bind(&root::_handle_receive, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
    }
    
    void root::_handle_receive(const boost::system::error_code& err, const size_t bytesReceived){

        if(err) return;
        
        node_message msg;
        msg.decode(&_buffer[0]);
        
        if(msg.is_valid()){
            switch(msg.get_type()){
                case NODE_PING_LIB:
                    //_handle_node_ping_lib(msg);
                    break;
                    
                case PRGM_GET_MEM:
                    _handle_prgm_get_mem(msg);
                    break;
                    
                case PRGM_MOV_TASKS:
                    _handle_prgm_move_tasks(msg);
                    break;
                    
                case HOST_EXEC_KERNEL:
                    _handle_host_exec_kernel(msg);
                    break;
                    
                case HOST_GET_MEM:
                    _handle_host_get_mem(msg);
                    break;
                    
                default:
                    break;
            }
        }
            
        _start_accept();        
    }
    
    void root::_handle_send(const boost::system::error_code& err, const size_t bytesSent){
        
    }
    
    /*void root::_handle_node_ping_lib(node_message& msg){
        std::cout << std::string(msg.get_data()) << std::endl;
        _node_map[std::string(msg.get_data())] = true;
    }*/
    
    //Get data requested from remote node and update local shared memory
    void root::_handle_prgm_get_mem(node_message& msg){
        //Decode message
        std::stringstream msg_stream;
        msg_stream << msg.get_data();
        
        boost::archive::text_iarchive ia(msg_stream);
        mob::set_mem data;
        ia >> data;
        
        //Set shared local memory
        bip::managed_shared_memory segment(bip::open_only, data.prgm_name.c_str());
        if(data.val_type == "float"){  
            auto res = segment.find<float>(data.var_name.c_str());
            float* val = res.first;
    
            (*(val+data.idx)) = atof(data.val.c_str());  
            
            //Flag as miss
            //gmem<float>* mem = (gmem<float>*)_allocated_mem.at(data.var_name);
            //mem->_locality_miss(data.node_name, data.idx);
        }
        else if(data.val_type == "float4"){
            auto res = segment.find<float4>(data.var_name.c_str());
            float4* val = res.first;
    
            (*(val+data.idx)) = float4(data.val); 
            
            //Flag as miss
            //gmem<float4>* mem = (gmem<float4>*)_allocated_mem.at(data.var_name);
            //mem->_locality_miss(data.node_name, data.idx);            
        }
        
        //Signal gmem that the data is updated
        _sig_remote_get(data.node_name, data.idx, data.var_name);   
    }
    
    void root::_handle_prgm_move_tasks(node_message& msg){
        std::cout << "_handle_prgm_move_tasks" << std::endl;
        
        //Decode message
        std::stringstream msgStream;
        msgStream << msg.get_data();
        
        boost::archive::text_iarchive ia(msgStream);
        node_task_data data;
        ia >> data; 
        
        bip::managed_shared_memory segment(bip::open_only, _prgm_name.c_str());      
        TaskList* mem = segment.find<TaskList>("task_list").first;        
        
        //Remove elements that we moved        
        for(auto idx : data.task_list){
            mem->erase(std::find(mem->begin(), mem->end(), idx));
        }
        
        //Reset moving status
        _sig_remote_move(data.task_list[0]);
    }
    
    void root::_handle_host_exec_kernel(node_message& msg){
        std::cout << "exec kernel" << std::endl;
        
        //Decode message
        std::stringstream msg_stream;
        msg_stream << msg.get_data();
        
        boost::archive::text_iarchive ia(msg_stream);
        prgm_kernel_data data;
        ia >> data;
        
        //Launch kernel
        if(data.prgm_name == _prgm_name){ 
            if(_kernel_map.find(data.kernel_name) != _kernel_map.end()){
                exec_kernel(data.kernel_name);
            }  
        }
        
        //Ping back
        mob::node_message msgOut(mob::NODE_PING_LIB);
        std::string node_name = asio::ip::host_name();
        msgOut.set_data(node_name.c_str(), node_name.length());
        
        std::pair<char*, size_t> msgPair = msgOut.encode();
        
        asio::ip::udp::resolver res(_service);
        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), data.host_name, boost::lexical_cast<std::string>(HOST_PORT));
        asio::ip::udp::endpoint ep = *res.resolve(query);
        
        //_socket.async_send_to(asio::buffer(msgPair.first, msgPair.second), ep, boost::bind(&root::_handle_send, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
        _socket.send_to(asio::buffer(msgPair.first, msgPair.second), ep);    
        
        delete[] msgPair.first;                                      
    }
    
    void root::_handle_host_get_mem(node_message& msg){
        std::cout << "host get mem" << std::endl;
        
        //Decode message
        std::stringstream msg_stream;
        msg_stream << msg.get_data();
        
        boost::archive::text_iarchive ia(msg_stream);
        prgm_var_data data;
        ia >> data;
        
        if(data.prgm_name == _prgm_name){
            boost::thread fetch_mem([=](){
                /*//Fetch from remote
                std::cout << "fetch" << std::endl;
                size_t end = 0;
                if(data.var_type == "float"){
                    gmem<float>* mem = (gmem<float>*)_allocated_mem.at(data.var_name);
                    mem->fetch();
                    end = mem->size();
                }
                else if(data.var_type == "float4"){
                    gmem<float4>* mem = (gmem<float4>*)_allocated_mem.at(data.var_name);
                    mem->fetch();  
                    end = mem->size();            
                }
                
                usleep(250000); //latency in memory propagation? this is a kludge either way
                
                _host_get_mem(data, end);*/
            gmem<float>* mem = (gmem<float>*)_allocated_mem.at(data.var_name);
            _host_get_mem(data, mem->size()); 
            }); 
                
        }       
    }
    
    void root::_host_get_mem(prgm_var_data data, size_t sz){
        std::cout << "_host_get_mem" << std::endl;
        //_host_get_mem_mtx.lock();
        
        //Get shared local memory
        bip::managed_shared_memory segment(bip::open_only, data.prgm_name.c_str());
        TaskList* task_list = segment.find<TaskList>("task_list").first;
 
        size_t block_size = floor(float(task_list->size())/16.0f);
        for(size_t i=0; i<16; i++){
            size_t start = i*block_size;
            size_t end = start+block_size;
        
            prgm_var_data data_out;
            //data_out.start = start;
            //data_out.end = end;
            
            if(data.var_type == "float"){
                auto res = segment.find<float>(data.var_name.c_str());
                float* val = res.first;
                
                //std::vector<float> out_vec(val + start, val + end);
                //std::vector<float> out_vec;
                for(size_t i=start; i<end; i++){
                    data_out.var_float.push_back(val[(*task_list)[i]]);
                    data_out.var_indices.push_back((*task_list)[i]);
                }
                
                //data_out.var_float = out_vec;
                data_out.var_type = "float";
            }
            else if(data.var_type == "float4"){
                auto res = segment.find<float4>(data.var_name.c_str());
                float4* val = res.first;
                
                //std::vector<float4> out_vec(val + start, val + end);
                //std::vector<float4> out_vec;
                for(size_t i=start; i<end; i++){
                    data_out.var_float4.push_back(val[(*task_list)[i]]);
                    data_out.var_indices.push_back((*task_list)[i]);
                }
                                
                //data_out.var_float4 = out_vec;
                data_out.var_type = "float4";            
            }
            
            
            data_out.host_name = asio::ip::host_name();
            data_out.prgm_name = data.prgm_name;
            data_out.var_name = data.var_name;
                       
            //Ping back
            //std::cout << "ping back" << std::endl;
            node_message msg_out(HOST_GET_MEM);
            
            std::stringstream msg_stream_out;
            boost::archive::text_oarchive oa(msg_stream_out);
            oa << data_out;
                    
            msg_out.set_data(msg_stream_out.str().c_str(), msg_stream_out.str().size());
            std::pair<char*, size_t> msgPair = msg_out.encode();
            
            asio::ip::udp::resolver resolver(_service);
            asio::ip::udp::resolver::query query(asio::ip::udp::v4(), data.host_name, boost::lexical_cast<std::string>(HOST_PORT));
            asio::ip::udp::endpoint ep = *resolver.resolve(query);
            
            //std::cout << "sending..." << std::endl;
            //_socket.async_send_to(asio::buffer(msgPair.first, msgPair.second), ep, boost::bind(&root::_handle_send, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
            _socket.send_to(asio::buffer(msgPair.first, msgPair.second), ep);
            
            delete[] msgPair.first;
            //std::cout << "end " << std::endl;  
        }      
        //_host_get_mem_mtx.unlock();
    }
    
    void root::_prgm_send_mem(node_message& msg){
        //_shared_mem_write.lock();
        //_mem_send_queue.push(msg);
        
        //if(_mem_send_queue.size() == 1){
            //boost::system::error_code error;
            //asio::ip::udp::socket broad_socket(_service);
            //broad_socket.open(asio::ip::udp::v4(), error);
            //if(!error){
                //broad_socket.set_option(asio::ip::udp::socket::reuse_address(true));
                //broad_socket.set_option(asio::socket_base::broadcast(true));
                
                std::pair<char*, size_t> msg_pair = msg.encode();
    
                asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), NODE_PORT);
                _socket.send_to(asio::buffer(msg_pair.first, msg_pair.second), senderEndpoint);
                //_socket.async_send_to(asio::buffer(msg_pair.first, msg_pair.second), senderEndpoint, boost::bind(&root::_handle_mem_send, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
                //broad_socket.close(error);
                
                delete[] msg_pair.first;
            //}
            //else{
            //   std::cout << "broadcast error" << std::endl;
            //}       
        //}     
        //_shared_mem_write.unlock();
    }
    
    void root::_handle_mem_send(const boost::system::error_code& err, const size_t bytesSent){
        //_shared_mem_write.lock();
        std::cout << "_handle_mem_send" << std::endl;
        if(_mem_send_queue.top() != nullptr){
            delete _mem_send_queue.top();
        }
        _mem_send_queue.pop();
        
        if(!_mem_send_queue.empty()){
            node_message* msg = _mem_send_queue.top();

            //boost::system::error_code error;
            //asio::ip::udp::socket broad_socket(_service);
            //broad_socket.open(asio::ip::udp::v4(), error);
            //if(!error){
                //.set_option(asio::ip::udp::socket::reuse_address(true));
                //broad_socket.set_option(asio::socket_base::broadcast(true));
                
                std::pair<char*, size_t> msg_pair = msg->encode();
    
                asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), NODE_PORT);
                //broad_socket.send_to(asio::buffer(msg_pair.first, msg_pair.second), senderEndpoint)
                _socket.async_send_to(asio::buffer(msg_pair.first, msg_pair.second), senderEndpoint, boost::bind(&root::_handle_mem_send, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
                //broad_socket.close(error);
                
                delete[] msg_pair.first;
            //}
            //else{
            //    std::cout << "broadcast error" << std::endl;
            //}  
        }
        //_shared_mem_write.unlock();
    }
    
    void root::_prgm_get_mem(node_message& msg){
        //boost::system::error_code error;
        //asio::ip::udp::socket broad_socket(_service);
        //broad_socket.open(asio::ip::udp::v4(), error);
        //if(!error){
            //broad_socket.set_option(asio::ip::udp::socket::reuse_address(true));
            //broad_socket.set_option(asio::socket_base::broadcast(true));
            
            std::pair<char*, size_t> msg_pair = msg.encode();

            bool try_again = true;
            
            while(try_again){
                try{
                    asio::ip::udp::endpoint ep(asio::ip::address_v4::broadcast(), NODE_PORT);
                    _socket.send_to(asio::buffer(msg_pair.first, msg_pair.second), ep);
                    //broad_socket.close(error);
                    try_again = false;
                }
                catch(...){
                    std::cout << "EXCEPTION" << std::endl;
                }
            }
                
            
            delete[] msg_pair.first;
        //}
        //else{
        //    std::cout << "broadcast error" << std::endl;
        //}           
    }
    
    void root::_prgm_mov_task(std::string to_node, size_t idx){
        std::cout << "_prgm_mov_task " << to_node << std::endl;
        
        node_message msg(PRGM_MOV_TASKS);
        
        node_task_data data;
        data.host_name = asio::ip::host_name();
        data.prgm_name = _prgm_name;
        data.task_list.push_back(idx);
        
        std::stringstream msg_stream;
        boost::archive::text_oarchive oa(msg_stream);
        oa << data;
                
        msg.set_data(msg_stream.str().c_str(), msg_stream.str().size());
        std::pair<char*, size_t> msg_pair = msg.encode();
        
        asio::ip::udp::resolver resolver(_service);
        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), to_node, boost::lexical_cast<std::string>(NODE_PORT));
        asio::ip::udp::endpoint ep = *resolver.resolve(query);
        
        _socket.send_to(asio::buffer(msg_pair.first, msg_pair.second), ep);
        
        delete[] msg_pair.first;        
    }
    
    void root::_kernel_started(std::string kernel){
        std::cout << "_kernel_started" << std::endl;
        
        boost::system::error_code error;
        asio::ip::udp::socket broad_socket(_service);
        broad_socket.open(asio::ip::udp::v4(), error);
        if(!error){
            broad_socket.set_option(asio::ip::udp::socket::reuse_address(true));
            broad_socket.set_option(asio::socket_base::broadcast(true));
            
            node_message msg(KERNEL_STARTED);
            
            kernel_status_data data;
            data.host_name = asio::ip::host_name();
            data.prgm_name = _prgm_name;
            data.kernel_name = kernel;
            data.status = false;
            
            std::stringstream msg_stream;
            boost::archive::text_oarchive oa(msg_stream);
            oa << data;
                        
            msg.set_data(msg_stream.str().c_str(), msg_stream.str().size());
            std::pair<char*, size_t> msg_pair = msg.encode();
            
            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), NODE_PORT);
            broad_socket.send_to(asio::buffer(msg_pair.first, msg_pair.second), senderEndpoint);
            broad_socket.close(error);
            
            delete[] msg_pair.first;
        }
        else{
            std::cout << "broadcast error" << std::endl;
        }        
    }
    
    void root::_kernel_finished(std::string kernel){
        boost::system::error_code error;
        asio::ip::udp::socket broad_socket(_service);
        broad_socket.open(asio::ip::udp::v4(), error);
        if(!error){
            broad_socket.set_option(asio::ip::udp::socket::reuse_address(true));
            broad_socket.set_option(asio::socket_base::broadcast(true));
            
            node_message msg(KERNEL_FINISHED);
            
            kernel_status_data data;
            data.host_name = asio::ip::host_name();
            data.prgm_name = _prgm_name;
            data.kernel_name = kernel;
            data.status = true;
            
            std::stringstream msg_stream;
            boost::archive::text_oarchive oa(msg_stream);
            oa << data;
                        
            msg.set_data(msg_stream.str().c_str(), msg_stream.str().size());
            std::pair<char*, size_t> msg_pair = msg.encode();
            
            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), NODE_PORT);
            broad_socket.send_to(asio::buffer(msg_pair.first, msg_pair.second), senderEndpoint);
            broad_socket.close(error);
            
            delete[] msg_pair.first;
        }
        else{
            std::cout << "broadcast error" << std::endl;
        }   
    }

    
    boost::signals2::connection root::_connect_remote_get(const remote_get_event& e){
        return _sig_remote_get.connect(e);
    }
    
    boost::signals2::connection root::_connect_remote_move(const remote_move_event& e){
        return _sig_remote_move.connect(e);
    }
        
}