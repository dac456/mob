#include "NodeClient.h"

namespace MobNode
{
    
    NodeClient::NodeClient(asio::io_service& service) : _socket(service){
        asio::ip::tcp::resolver resolver(service);
        asio::ip::tcp::resolver::query query("127.0.0.1", boost::lexical_cast<std::string>(9001));
        
        asio::ip::tcp::resolver::iterator itr = resolver.resolve(query);
        asio::ip::tcp::endpoint endPoint = *itr;
        
        _socket.async_connect(endPoint, boost::bind(&NodeClient::onConnect, this, asio::placeholders::error, itr++));
    }
    
    void NodeClient::onConnect(const boost::system::error_code& err, asio::ip::tcp::resolver::iterator itr){
        if(!err){
            _socket.async_receive(asio::buffer(_buffer, 512), boost::bind(&NodeClient::onReceive, this, asio::placeholders::error));
        }
        else{
            //TODO: try again
        }
    }
    
    void NodeClient::onReceive(const boost::system::error_code& err){
        if(!err){
            std::cout << _buffer << std::endl;
            
            _socket.async_receive(asio::buffer(_buffer, 512), boost::bind(&NodeClient::onReceive, this, asio::placeholders::error));
        }
        else{
            _socket.close();
        }
    }

}