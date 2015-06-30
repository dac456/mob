#include "NodeServer.h"
#include "NodeConnection.h"

namespace MobNode
{

    NodeServer::NodeServer(asio::io_service& service) : _acc(service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 9001)){
        _startAccept();
    }
    
    NodeServer::~NodeServer(){
        
    }
    
    void NodeServer::_startAccept(){
        NodeConnectionPtr session = std::make_shared<NodeConnection>(_acc.get_io_service());
        _acc.async_accept(session->getSocket(), boost::bind(&NodeServer::_handleAccept, this, session, asio::placeholders::error));
    }
    
    void NodeServer::_handleAccept(NodeConnectionPtr session, const boost::system::error_code& err){
        if(err) return;
        
        asio::async_write(session->getSocket(), asio::buffer("hello world"), 
            boost::bind(&NodeConnection::_handleWrite, session, asio::placeholders::error, asio::placeholders::bytes_transferred));
            
        _startAccept();
    }

}