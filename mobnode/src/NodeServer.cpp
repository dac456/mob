#include "NodeServer.h"
#include "NodeConnection.h"
#include "GMem.h"

namespace MobNode
{

    NodeServer::NodeServer(asio::io_service& service) : _socket(service, asio::ip::udp::endpoint(asio::ip::udp::v4(), NODE_PORT)){
        _service = &service;
        _startAccept();
    }
    
    NodeServer::~NodeServer(){
        
    }
    
    void NodeServer::_startAccept(){
        //NodeConnectionPtr session = std::make_shared<NodeConnection>(_acc.get_io_service());
        _socket.async_receive_from(asio::buffer(_buffer), _senderEndpoint, boost::bind(&NodeServer::_handleReceive, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
    }
    
    void NodeServer::_handleReceive(const boost::system::error_code& err, const size_t bytesReceived){
        if(err) return;
        
        mob::node_message msg;
        msg.decode(_buffer);
        
        if(msg.is_valid()){
            switch(msg.get_type()){
                case mob::NODE_PING:
                    _handleMsgPing(msg);
                    break;
                    
                case mob::NODE_PING_LIB:
                    _handleMsgPingLib(msg);
                    break;
                    
                case mob::PRGM_SET_MEM:
                    _handleMsgPrgmSetMem(msg);
                    break;
                    
                case mob::PRGM_GET_MEM:
                    _handleMsgPrgmGetMem(msg);
                    break;
                    
                case mob::NODE_START_PRGM:
                    _handleMsgStartPrgm(msg);
                    break;
                    
                default:
                    break;
            }
        }
        
        //_socket.async_send_to(asio::buffer("hello world"), _senderEndpoint, boost::bind(&NodeServer::_handleSend, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
            
        _startAccept();
    }

    void NodeServer::_handleSend(const boost::system::error_code& err, const size_t bytesSent){
        //std::cout << bytesSent << std::endl;
    }
    
    
    void NodeServer::_handleMsgPing(mob::node_message& msg){
        std::cout << "PING from: " << std::string(msg.get_data()) << std::endl;
        
        if(std::string(msg.get_data()) != std::string(asio::ip::host_name())){
            bool storedPrev = false;
            if(_nodeMap.count(std::string(msg.get_data())) > 0){
                storedPrev = true;
            }
            
            _nodeMap[std::string(msg.get_data())] = true;
            
            if(!storedPrev){
                //We haven't heard from this node before, send a ping back
                
                mob::node_message msgOut(mob::NODE_PING);
                std::string nodeName = asio::ip::host_name();
                msgOut.set_data(nodeName.c_str(), nodeName.length());
                
                std::pair<char*, size_t> msgPair = msgOut.encode();
                
                asio::ip::udp::resolver res(*_service);
                asio::ip::udp::resolver::query query(asio::ip::udp::v4(), std::string(msg.get_data()), boost::lexical_cast<std::string>(NODE_PORT));
                asio::ip::udp::endpoint ep = *res.resolve(query);
                
                _socket.async_send_to(asio::buffer(msgPair.first, msgPair.second), ep, boost::bind(&NodeServer::_handleSend, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
            }
        }
    }
    
    void NodeServer::_handleMsgPingLib(mob::node_message& msg){
        mob::node_message msgOut(mob::NODE_PING_LIB);
        std::string nodeName = asio::ip::host_name();
        msgOut.set_data(nodeName.c_str(), nodeName.length());
        
        std::pair<char*, size_t> msgPair = msgOut.encode();
        
        asio::ip::udp::resolver res(*_service);
        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), std::string(msg.get_data()), boost::lexical_cast<std::string>(9003));
        asio::ip::udp::endpoint ep = *res.resolve(query);
        
        _socket.async_send_to(asio::buffer(msgPair.first, msgPair.second), ep, boost::bind(&NodeServer::_handleSend, this, asio::placeholders::error, asio::placeholders::bytes_transferred));        
    }
    
    void NodeServer::_handleMsgPrgmSetMem(mob::node_message& msg){
        //Decode message
        std::stringstream msg_stream;
        msg_stream << msg.get_data();
        
        boost::archive::text_iarchive ia(msg_stream);
        mob::set_mem mem;
        ia >> mem;
        
        //Find memory in shared pool and set
        bip::managed_shared_memory segment(bip::open_only, mem.prgm_name.c_str());
        std::pair<float*, bip::managed_shared_memory::size_type> res;
        
        res = segment.find<float>(mem.var_name.c_str());
        float* val = res.first;

        (*(val+mem.idx)) = atof(mem.val.c_str());
    }
    
    void NodeServer::_handleMsgPrgmGetMem(mob::node_message& msg){
        //Decode message
        std::stringstream msg_stream;
        msg_stream << msg.get_data();
        
        boost::archive::text_iarchive ia(msg_stream);
        mob::set_mem mem;
        ia >> mem;
        
        //Find memory in shared pool
        bip::managed_shared_memory segment(bip::open_only, mem.prgm_name.c_str());
        std::pair<float*, bip::managed_shared_memory::size_type> res;
        
        res = segment.find<float>(mem.var_name.c_str());
        float* val = res.first; 
        
        //mem.val = (*(val+mem.idx));
        std::stringstream val_str;
        val_str << (*(val+mem.idx));
        mem.val = val_str.str();   
        
        //Re-serialize data
        std::stringstream msg_stream_out;
        boost::archive::text_oarchive oa(msg_stream_out);
        oa << mem;        
        
        //Send back
        mob::node_message msgOut(mob::PRGM_GET_MEM);
        std::string nodeName = asio::ip::host_name();        
        
        msgOut.set_data(msg_stream_out.str().c_str(), msg_stream_out.str().size());
        std::pair<char*, size_t> msgPair = msgOut.encode();
        
        asio::ip::udp::resolver resolver(*_service);
        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), std::string(mem.node_name), boost::lexical_cast<std::string>(9002));
        asio::ip::udp::endpoint ep = *resolver.resolve(query);
        
        _socket.async_send_to(asio::buffer(msgPair.first, msgPair.second), ep, boost::bind(&NodeServer::_handleSend, this, asio::placeholders::error, asio::placeholders::bytes_transferred));                
    }
    
    void NodeServer::_handleMsgStartPrgm(mob::node_message& msg){
        std::cout << "start_prgm" << std::endl;
        
        //Decode message
        std::stringstream msg_stream;
        msg_stream << msg.get_data();
        
        boost::archive::text_iarchive ia(msg_stream);
        prgm_data mem;
        ia >> mem;
        
        //Start named process
        std::vector<std::string> args;
        args.push_back(mem.prgm_name); 
        args.push_back("--numtasks");
        args.push_back(std::to_string(mem.num_tasks));
        
        proc::context ctx; 
        ctx.stdout_behavior = proc::capture_stream();         
        
        proc::child c = proc::launch(mem.prgm_name, args, ctx);
        
        //TODO: run thread to capture stdout
        proc::pistream &is = c.get_stdout(); 
        std::string line; 
        while (std::getline(is, line)) 
            std::cout << line << std::endl;         
        
        //Store process
        proc_info info;
        info.process = &c;
        info.num_tasks = mem.num_tasks;
        info.name = mem.prgm_name;   
        info.running = true;
        
        _programMap[mem.prgm_name] = info;
        
        //TODO:
        // determine how many available nodes we have, start program on others, and divides tasks evenly
        // node_set_tasks(prgm,tasks) -> prgm_set_tasks
    }

}