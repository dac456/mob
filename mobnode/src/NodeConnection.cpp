#include "NodeConnection.h"

namespace MobNode
{

    NodeConnection::NodeConnection(asio::io_service& service) : _socket(service){
    }
    
    asio::ip::tcp::socket& NodeConnection::getSocket(){
        return _socket;
    }
    
    std::shared_ptr<NodeConnection> NodeConnection::create(asio::io_service& service){
        //return std::make_shared<NodeConnection>(service);
        return std::shared_ptr<NodeConnection>(new NodeConnection(service));
    }
    
    void NodeConnection::_handleWrite(const boost::system::error_code& err, size_t bytesTransferred){
        
    }

}