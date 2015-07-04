#include "NodeClient.h"
#include "NodeMessage.h"

namespace MobNode
{
    
    NodeClient::NodeClient(asio::io_service& service, std::string addr) : _socket(service){
        asio::ip::udp::resolver resolver(service);
        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), addr, boost::lexical_cast<std::string>(9001));

        asio::ip::udp::endpoint receiverEndpoint = *resolver.resolve(query);

        //asio::ip::udp::socket sck(service);
        _socket.open(asio::ip::udp::v4());
        //_socket.async_send_to(asio::buffer("hello server"), receiverEndpoint, boost::bind(&NodeClient::_onSend, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
        
        //_socket.async_connect(receiverEndpoint, boost::bind(&NodeClient::onConnect, this, asio::placeholders::error, itr++));
        //TODO: broadcast and find all nodes on network
        boost::system::error_code error;
        asio::ip::udp::socket broadSocket(service);
        broadSocket.open(asio::ip::udp::v4(), error);
        if(!error){
            broadSocket.set_option(asio::ip::udp::socket::reuse_address(true));
            broadSocket.set_option(asio::socket_base::broadcast(true));
            
            NodeMessage msg(NODE_PING);
            msg.setData("hello", 5);
            
            std::pair<char*, size_t> msgPair = msg.encode();

            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), 9001);
            broadSocket.send_to(asio::buffer(msgPair.first, msgPair.second), senderEndpoint);
            broadSocket.close(error);
        }
        else{
            std::cout << "broadcast error" << std::endl;
        }

    }

    NodeClient::~NodeClient(){
        _socket.close();
    }

    void NodeClient::_onSend(const boost::system::error_code& err, const size_t bytesSent){
        _socket.close();
    }

}