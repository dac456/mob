#include "NodeServer.h"
#include "NodeConnection.h"

namespace MobNode
{

    NodeServer::NodeServer(asio::io_service& service) : _socket(service, asio::ip::udp::endpoint(asio::ip::udp::v4(), 9001)){
        _startAccept();
    }
    
    NodeServer::~NodeServer(){
        
    }
    
    void NodeServer::_startAccept(){
        //NodeConnectionPtr session = std::make_shared<NodeConnection>(_acc.get_io_service());
        _socket.async_receive_from(asio::buffer(_buffer), _senderEndpoint, boost::bind(&NodeServer::_handleReceive, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
    }
    
    void NodeServer::_handleReceive(const boost::system::error_code& err, const size_t bytesReceived){
        //std::cout << err << std::endl;
        if(err) return;

        std::cout << _buffer << std::endl;
        
        //asio::async_write(session->getSocket(), asio::buffer("hello world"), 
        //    boost::bind(&NodeConnection::_handleWrite, session, asio::placeholders::error, asio::placeholders::bytes_transferred));
        _socket.async_send_to(asio::buffer("hello world"), _senderEndpoint, boost::bind(&NodeServer::_handleSend, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
            
        _startAccept();
    }

    void NodeServer::_handleSend(const boost::system::error_code& err, const size_t bytesSend){
        
    }

}