#include "NodeClient.h"
#include "NodeMessage.h"

namespace MobNode
{
    
    NodeClient::NodeClient(asio::io_service& service, std::string addr) : _socket(service){
        asio::ip::udp::resolver resolver(service);
        asio::ip::udp::resolver::query query(asio::ip::udp::v4(), addr, boost::lexical_cast<std::string>(NODE_PORT));

        asio::ip::udp::endpoint receiverEndpoint = *resolver.resolve(query);

        _socket.open(asio::ip::udp::v4());
        
        //Broadcast a NODE_PING message and find all nodes on network
        boost::system::error_code error;
        asio::ip::udp::socket broadSocket(service);
        broadSocket.open(asio::ip::udp::v4(), error);
        if(!error){
            broadSocket.set_option(asio::ip::udp::socket::reuse_address(true));
            broadSocket.set_option(asio::socket_base::broadcast(true));
            
            NodeMessage msg(NODE_PING);
            
            std::string nodeName = asio::ip::host_name();
            msg.setData(nodeName.c_str(), nodeName.length());
            
            std::pair<char*, size_t> msgPair = msg.encode();

            asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), NODE_PORT);
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