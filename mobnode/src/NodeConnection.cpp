#include "NodeConnection.h"

namespace MobNode
{

    NodeConnection::NodeConnection(asio::io_service& service) : _socket(service){
    }
    
    asio::ip::tcp::socket& NodeConnection::getSocket(){
        return _socket;
    }
    
    void NodeConnection::_handleWrite(const boost::system::error_code& err, size_t bytesTransferred){
        
    }

}