#include "Root.h"
#include "NodeMessage.h"
#include "GMem.h"

namespace mob
{
    
    //TODO: program ports should be incremented to allow multiple programs per node
    root::root() : _socket(_service, asio::ip::udp::endpoint(asio::ip::udp::v4(), 9002)){
        _waiting_for_tasks = true;
    }
    
    root::~root(){
        
    }
    
    void root::mob_init(int argc, char* argv[]){
        _prgm_name = argv[0];
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
        bip::shared_memory_object::remove(_prgm_name.c_str()); //Clear anything left from previous execution
        bip::shared_memory_object::remove("task_list");
        
        try{
            bip::managed_shared_memory segment(bip::open_or_create, _prgm_name.c_str(), 65536);
            segment.construct<TaskList>("task_list")(segment.get_segment_manager());
        }
        catch(...){
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
            
            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), 9001);
            broad_socket.send_to(asio::buffer(msg_pair.first, msg_pair.second), senderEndpoint);
            broad_socket.close(error);
        }
        else{
            std::cout << "broadcast error" << std::endl;
        }          
        
        //Start an async server so we can talk to the network
        _start_accept();
        boost::thread srv(boost::bind(&asio::io_service::run, &_service));
    }

    void root::mob_kill(){
        _socket.close();
        bip::shared_memory_object::remove(_prgm_name.c_str());
    }
    
    std::string root::get_name(){
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
        msg.decode(_buffer);
        
        if(msg.is_valid()){
            switch(msg.get_type()){
                case NODE_PING_LIB:
                    //_handle_node_ping_lib(msg);
                    break;
                    
                case PRGM_GET_MEM:
                    _handle_prgm_get_mem(msg);
                    break;
                    
                default:
                    break;
            }
        }
            
        _start_accept();        
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
        mob::set_mem mem;
        ia >> mem;
        
        //Set shared local memory
        bip::managed_shared_memory segment(bip::open_only, mem.prgm_name.c_str());
        std::pair<float*, bip::managed_shared_memory::size_type> res;
        
        res = segment.find<float>(mem.var_name.c_str());
        float* val = res.first;

        (*(val+mem.idx)) = atof(mem.val.c_str());     
        
        //Signal gmem that the data is updated
        _sig_remote_get(mem.idx);   
    }
    
    void root::_prgm_send_mem(node_message& msg){
        boost::system::error_code error;
        asio::ip::udp::socket broad_socket(_service);
        broad_socket.open(asio::ip::udp::v4(), error);
        if(!error){
            broad_socket.set_option(asio::ip::udp::socket::reuse_address(true));
            broad_socket.set_option(asio::socket_base::broadcast(true));
            
            std::pair<char*, size_t> msg_pair = msg.encode();

            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), 9001);
            broad_socket.send_to(asio::buffer(msg_pair.first, msg_pair.second), senderEndpoint);
            broad_socket.close(error);
        }
        else{
            std::cout << "broadcast error" << std::endl;
        }            
    }
    
    void root::_prgm_get_mem(node_message& msg){
        boost::system::error_code error;
        asio::ip::udp::socket broad_socket(_service);
        broad_socket.open(asio::ip::udp::v4(), error);
        if(!error){
            broad_socket.set_option(asio::ip::udp::socket::reuse_address(true));
            broad_socket.set_option(asio::socket_base::broadcast(true));
            
            std::pair<char*, size_t> msg_pair = msg.encode();

            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), 9001);
            broad_socket.send_to(asio::buffer(msg_pair.first, msg_pair.second), senderEndpoint);
            broad_socket.close(error);
        }
        else{
            std::cout << "broadcast error" << std::endl;
        }           
    }

    
    boost::signals2::connection root::_connect_remote_get(const remote_get_event& e){
        return _sig_remote_get.connect(e);
    }
        
}