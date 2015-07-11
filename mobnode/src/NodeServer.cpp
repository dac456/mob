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
        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), std::string(msg.get_data()), boost::lexical_cast<std::string>(9002));
        asio::ip::udp::endpoint ep = *res.resolve(query);
        
        _socket.async_send_to(asio::buffer(msgPair.first, msgPair.second), ep, boost::bind(&NodeServer::_handleSend, this, asio::placeholders::error, asio::placeholders::bytes_transferred));        
    }
    
    void NodeServer::_handleMsgPrgmSetMem(mob::node_message& msg){
        std::cout << "set_mem" << std::endl;
        
        std::stringstream msg_stream;
        msg_stream << msg.get_data();
        
        boost::archive::text_iarchive ia(msg_stream);
        mob::set_mem mem;
        ia >> mem;
        
        bip::managed_shared_memory segment(bip::open_only, mem.prgm_name.c_str());
        std::pair<float*, bip::managed_shared_memory::size_type> res;
        
        res = segment.find<float>(mem.var_name.c_str());
        float* val = res.first;

        (*(val+mem.idx)) = atof(mem.val.c_str());
    }

}