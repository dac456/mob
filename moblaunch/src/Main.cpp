#include "LaunchCommon.h"
#include "NodeMessage.h"

struct prgm_data{
    
    std::string prgm_name;
    size_t num_tasks;
    
    template<typename Archive>
    void serialize(Archive& ar, const unsigned int version){
        ar & prgm_name;
        ar & num_tasks;
    }
};

int main(int argc, char* argv[])
{
    asio::io_service service;
    boost::thread srv(boost::bind(&asio::io_service::run, &service));
    
    std::string prgmName = "";

    int nextParam = 0;
    for(int i=0; i<argc; i++){
        if(strstr(argv[i], "--") == nullptr){
            if(nextParam == 1){
                prgmName = std::string(argv[i]);
            }
        }
        else{     
            if(strcmp(argv[i], "--p") == 0){
                nextParam = 1;
            }          
        }            
    }
    
    /* TODO: 
     * broadcast a node_ping and get a list of available nodes
     * choose a node (at random for now) to start program and initially assign all tasks there
     *  -> add logic in libmob such that if tasks are not provided at start it should wait for them
     *  -> node needs some way to tell how many tasks there are
     *  ->  could report back to node somehow
     *  ->  could provide manifest for program - simpler for now - could be supplied by moblaunch
     */
    
    //Broadcast and find nodes
    boost::system::error_code error;
    asio::ip::udp::socket broad_socket(service);
    broad_socket.open(asio::ip::udp::v4(), error);
    if(!error){
        broad_socket.set_option(asio::ip::udp::socket::reuse_address(true));
        broad_socket.set_option(asio::socket_base::broadcast(true));
        
        mob::node_message msg(mob::NODE_PING_LIB);
        
        std::string node_name = asio::ip::host_name();
        msg.set_data(node_name.c_str(), node_name.length());
        
        std::pair<char*, size_t> msg_pair = msg.encode();

        asio::ip::udp::endpoint senderEndpoint(asio::ip::address_v4::broadcast(), NODE_PORT);
        broad_socket.send_to(asio::buffer(msg_pair.first, msg_pair.second), senderEndpoint);
        broad_socket.close(error);
    }
    else{
        std::cout << "broadcast error" << std::endl;
    }
    
    //Wait for first response
    char rec_buf[1024];
    asio::ip::udp::endpoint sender_endpoint;
    asio::ip::udp::socket socket(service, asio::ip::udp::endpoint(asio::ip::udp::v4(), LNCH_PORT));
    socket.receive_from(asio::buffer(rec_buf), sender_endpoint);

    //Get node
    std::string first_node = "";
    
    mob::node_message msg;
    msg.decode(rec_buf);
    
    if(msg.is_valid()){
        if(msg.get_type() == mob::NODE_PING_LIB){
            first_node = std::string(msg.get_data());
        }
    }
    
    //Start program
    mob::node_message msgOut(mob::LAUNCH_PRGM);
    
    prgm_data data;
    data.prgm_name = prgmName;
    data.num_tasks = 4; //TODO: don't hardcode
    
    std::stringstream dataStream;
    boost::archive::text_oarchive oa(dataStream);
    oa << data;
    
    msgOut.set_data(dataStream.str().c_str(), dataStream.str().size());
    std::pair<char*, size_t> msgPair = msgOut.encode();
    
    asio::ip::udp::resolver res(service);
    asio::ip::udp::resolver::query query(asio::ip::udp::v4(), first_node, boost::lexical_cast<std::string>(NODE_PORT));
    asio::ip::udp::endpoint ep = *res.resolve(query);
    
    socket.send_to(asio::buffer(msgPair.first, msgPair.second), ep);

}