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
    
    void NodeServer::_launchProcess(std::string path, std::vector<std::string> args, proc::context ctx){
        proc::child c = proc::launch(path, args, ctx);
        
        for(;;){
            proc::pistream &is = c.get_stdout(); 
            std::string line; 
            while (std::getline(is, line)) 
                std::cout << line << std::endl;    
        }          
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
                    
                case mob::LAUNCH_PRGM:
                    _handleMsgLaunchPrgm(msg);
                    break;
                    
                case mob::NODE_START_PRGM:
                    _handleMsgStartPrgm(msg);
                    break;
                    
                case mob::NODE_SET_TASKS:
                    _handleMsgSetTasks(msg);
                    break;
                
                case mob::PRGM_STARTED:
                    _handleMsgPrgmStarted(msg);
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
        
        _nodeMap[std::string(msg.get_data())] = true;        
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
    
    void NodeServer::_handleMsgLaunchPrgm(mob::node_message& msg){
        std::cout << "launch_prgm" << std::endl;
        
        //Decode message
        std::stringstream msg_stream;
        msg_stream << msg.get_data();
        
        boost::archive::text_iarchive ia(msg_stream);
        prgm_data mem;
        ia >> mem;
        
        std::cout << "launch_prgm: " << mem.prgm_name << std::endl;
        if(_programMap.find(mem.prgm_name) == _programMap.end()){
            //Start named process
            std::vector<std::string> args;
            args.push_back(mem.prgm_name); 
            args.push_back("--numtasks");
            args.push_back(std::to_string(mem.num_tasks));
            
            proc::context ctx; 
            ctx.stdout_behavior = proc::capture_stream();         
            
            std::cout << "launch prgm: starting proc" << std::endl;
            std::stringstream path;
            path /*<< "./"*/ << mem.prgm_name;
            //proc::child c = proc::launch(path.str(), args, ctx);
            
            /*proc::status s = c.wait();
            if(s.exited()){
                std::cout << "err" << std::endl;
            }*/
            
            boost::thread newproc(boost::bind(&NodeServer::_launchProcess, this, path.str(), args, ctx));        
            
            //Store process
            proc_info info;
            //info.process = &c;
            info.num_tasks = mem.num_tasks;
            info.name = mem.prgm_name;
            info.started = true;
            
            for(auto& node : _nodeMap){
                info.running_on_node[node.first] = false;
            }   
            
            _programMap[mem.prgm_name] = info;
            
            //TODO:
            // determine how many available nodes we have, start program on others, and divides tasks evenly
            // node_set_tasks(prgm,tasks) -> prgm_set_tasks
            
            //Broadcast program start
            //TODO: possibly iterate over all nodes for p2p transfer instead?
            boost::system::error_code error;
            asio::ip::udp::socket broad_socket(*_service);
            broad_socket.open(asio::ip::udp::v4(), error);
            if(!error){
                broad_socket.set_option(asio::ip::udp::socket::reuse_address(true));
                broad_socket.set_option(asio::socket_base::broadcast(true));
                
                mob::node_message msgOut(mob::NODE_START_PRGM);
                
                prgm_data data;
                data.prgm_name = mem.prgm_name;
                data.num_tasks = mem.num_tasks;
                
                std::stringstream dataStream;
                boost::archive::text_oarchive oa(dataStream);
                oa << data;
                
                msgOut.set_data(dataStream.str().c_str(), dataStream.str().size());
        
                std::pair<char*, size_t> msg_pair = msgOut.encode();
        
                asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), 9001);
                broad_socket.send_to(asio::buffer(msg_pair.first, msg_pair.second), senderEndpoint);
                broad_socket.close(error);
            }
            else{
                std::cout << "broadcast error" << std::endl;
            }
            
            //Initial task assignment
            size_t nodeCount = _nodeMap.size();
            std::vector<size_t> taskAssign[nodeCount];
            
            size_t blockSize = floor((float)(mem.num_tasks)/(float)(nodeCount));
            
            //TODO: check for odd number of tasks
            for(size_t i=0; i<nodeCount; i++){
                
                size_t start = i*blockSize;
                for(size_t j=start; j<start+blockSize; j++){
                    taskAssign[i].push_back(j);
                }
            }
            
            //Set tasks for each node
            size_t nodeIdx = 0;
            for(auto& node : _nodeMap){
                std::cout << "set task for " << node.first << std::endl;
                mob::node_message msgOut(mob::NODE_SET_TASKS);
                
                node_task_data msgData;
                msgData.prgm_name = mem.prgm_name;
                msgData.task_list = taskAssign[nodeIdx];
                
                std::stringstream msgStream;
                boost::archive::text_oarchive oa(msgStream);
                oa << msgData;                

                msgOut.set_data(msgStream.str().c_str(), msgStream.str().size());
                std::pair<char*, size_t> msgPair = msgOut.encode();
                
                asio::ip::udp::resolver resolver(*_service);
                asio::ip::udp::resolver::query query(asio::ip::udp::v4(), node.first, boost::lexical_cast<std::string>(9001));
                asio::ip::udp::endpoint ep = *resolver.resolve(query);
                
                _socket.async_send_to(asio::buffer(msgPair.first, msgPair.second), ep, boost::bind(&NodeServer::_handleSend, this, asio::placeholders::error, asio::placeholders::bytes_transferred));                  
            
                nodeIdx++;
            }
        }        
    }
    
    void NodeServer::_handleMsgStartPrgm(mob::node_message& msg){
        std::cout << "start_prgm" << std::endl;
        
        //Decode message
        std::stringstream msg_stream;
        msg_stream << msg.get_data();
        
        boost::archive::text_iarchive ia(msg_stream);
        prgm_data mem;
        ia >> mem;
        
        if(_programMap.find(mem.prgm_name) == _programMap.end() || _programMap.at(mem.prgm_name).started == false){
            //Start named process
            std::vector<std::string> args;
            args.push_back(mem.prgm_name); 
            args.push_back("--numtasks");
            args.push_back(std::to_string(mem.num_tasks));
            
            proc::context ctx; 
            ctx.stdout_behavior = proc::capture_stream();         
            
            std::stringstream path;
            path /*<< "./"*/ << mem.prgm_name;
            //proc::child c = proc::launch(path.str(), args, ctx);
            
            /*proc::status s = c.wait();
            if(s.exited()){
                std::cout << "err" << std::endl;
            }*/
            
            /*boost::thread output([&](){
                proc::child c = proc::launch(path.str(), args, ctx);
                for(;;){
                    proc::pistream &is = c.get_stdout(); 
                    std::string line; 
                    while (std::getline(is, line)) 
                        std::cout << line << std::endl;    
                }
            });*/
            boost::thread newproc(boost::bind(&NodeServer::_launchProcess, this, path.str(), args, ctx));      
            
            //Store process
            proc_info info;
            //info.process = &c;
            info.num_tasks = mem.num_tasks;
            info.name = mem.prgm_name;
            info.started = true;   
            
           for(auto& node : _nodeMap){
                info.running_on_node[node.first] = false;
            }              
            
            _programMap[mem.prgm_name] = info;
        }        
    }    
    
    void NodeServer::_handleMsgSetTasks(mob::node_message& msg){
        std::cout << "set_tasks" << std::endl;
        
        //Decode message
        std::stringstream msgStream;
        msgStream << msg.get_data();
        
        boost::archive::text_iarchive ia(msgStream);
        node_task_data data;
        ia >> data;  
        
        //Set tasks in the program map
        _programMap.at(data.prgm_name).task_indices.clear();
        _programMap.at(data.prgm_name).task_indices = data.task_list;
        
        //Bounce the tasks to the correct program on each node
        boost::thread sendTasks([=](){
            size_t nodeIdx = 0;
            for(auto& node : _nodeMap){
                if(node.second){
                    while(!_programMap[data.prgm_name].running_on_node.at(node.first)); //block until running
                    
                    if(_programMap[data.prgm_name].running_on_node.at(node.first)){
                        mob::node_message msgOut(mob::PRGM_SET_TASKS);
                        
                        std::stringstream dataStream;
                        boost::archive::text_oarchive oa(dataStream);
                        oa << data;
                        
                        msgOut.set_data(dataStream.str().c_str(), dataStream.str().size());
                        std::pair<char*, size_t> msgPair = msgOut.encode();
                        
                        asio::ip::udp::resolver resolver(*_service);
                        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), node.first, boost::lexical_cast<std::string>(9002));
                        asio::ip::udp::endpoint ep = *resolver.resolve(query);
                        
                        _socket.async_send_to(asio::buffer(msgPair.first, msgPair.second), ep, boost::bind(&NodeServer::_handleSend, this, asio::placeholders::error, asio::placeholders::bytes_transferred));                  
                    
                        nodeIdx++;
                    }
                }
            }
        });
    }
    
    void NodeServer::_handleMsgPrgmStarted(mob::node_message& msg){
        std::cout << "prgm_started" << std::endl;
        
        //Decode message
        std::stringstream msgStream;
        msgStream << msg.get_data();
        
        boost::archive::text_iarchive ia(msgStream);
        prgm_started_data data;
        ia >> data;
        
        //Update running status
        _programMap[data.prgm_name].running_on_node[data.node_name] = true;          
    }

}