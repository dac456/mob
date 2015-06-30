#include "NodeServer.h"
#include "NodeClient.h"

int main(int argc, char* argv[]){
    asio::io_service serverService;
    asio::io_service clientService;
    
    std::shared_ptr<MobNode::NodeServer> server = std::make_shared<MobNode::NodeServer>(serverService);
    boost::thread srv(boost::bind(&asio::io_service::run, &serverService));
    
    std::shared_ptr<MobNode::NodeClient> client = std::make_shared<MobNode::NodeClient>(clientService);
    boost::thread cli(boost::bind(&asio::io_service::run, &clientService));
    
    //srv.join();
    //cli.join();
    for(;;){
        
    }
    
    return 0;
}