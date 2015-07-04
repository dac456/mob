#include "NodeServer.h"
#include "NodeConnection.h"
#include "NodeMessage.h"

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
        
        NodeMessage msg;
        msg.decode(_buffer);
        
        if(msg.isValid()){
            switch(msg.getType()){
                case NODE_PING:
                    _handleMsgPing(msg);
                    break;
                    
                default:
                    break;
            }
        }
        
        //_socket.async_send_to(asio::buffer("hello world"), _senderEndpoint, boost::bind(&NodeServer::_handleSend, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
            
        _startAccept();
    }

    void NodeServer::_handleSend(const boost::system::error_code& err, const size_t bytesSend){
        
    }
    
    
    void NodeServer::_handleMsgPing(NodeMessage& msg){
        std::cout << "PING from: " << std::string(msg.getData()) << std::endl;
        
        if(std::string(msg.getData()) != std::string(asio::ip::host_name())){
            bool storedPrev = false;
            if(_nodeMap.count(std::string(msg.getData())) > 0){
                storedPrev = true;
            }
            
            _nodeMap[std::string(msg.getData())] = true;
            
            if(!storedPrev){
                //We haven't heard from this node before, send a ping back
                
                NodeMessage msgOut(NODE_PING);
                std::string nodeName = asio::ip::host_name();
                msgOut.setData(nodeName.c_str(), nodeName.length());
                
                std::pair<char*, size_t> msgPair = msgOut.encode();
                
                asio::ip::udp::resolver res(*_service);
                asio::ip::udp::resolver::query query(asio::ip::udp::v4(), std::string(msg.getData()), boost::lexical_cast<std::string>(NODE_PORT));
                asio::ip::udp::endpoint ep = *res.resolve(query);
                
                _socket.async_send_to(asio::buffer(msgPair.first, msgPair.second), ep, boost::bind(&NodeServer::_handleSend, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
            }
        }
    }

}