#include "NodeServer.h"
#include "NodeConnection.h"
#include "NodeMessage.h"

namespace MobNode
{

    NodeServer::NodeServer(asio::io_service& service) : _socket(service, asio::ip::udp::endpoint(asio::ip::udp::v4(), 9001)){
        _startAccept();
        std::cout << asio::ip::host_name() << std::endl;
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
            std::cout << std::string(msg.getData()) << std::endl;
        }
        
        _socket.async_send_to(asio::buffer("hello world"), _senderEndpoint, boost::bind(&NodeServer::_handleSend, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
            
        _startAccept();
    }

    void NodeServer::_handleSend(const boost::system::error_code& err, const size_t bytesSend){
        
    }

}