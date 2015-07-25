#include "NodeServer.h"
#include "NodeClient.h"

int main(int argc, char* argv[]){
    std::cout << "Starting mobnode on " << asio::ip::host_name() << "..." << std::endl;
    
    asio::io_service serverService;
    asio::io_service clientService;
    
    std::shared_ptr<MobNode::NodeServer> server = std::make_shared<MobNode::NodeServer>(serverService);
    for(size_t i=0; i<10; i++){
        boost::thread srv(boost::bind(&asio::io_service::run, &serverService));
    }
    
    //TODO: create for each node we want to talk to?
    //      or have on Client wrap all outgoing comm. ?
    std::shared_ptr<MobNode::NodeClient> client = std::make_shared<MobNode::NodeClient>(clientService, "127.0.0.1");
    boost::thread cli(boost::bind(&asio::io_service::run, &clientService));
    
    //srv.join();
    //cli.join();

    for(;;){
        #ifdef MOB_PLATFORM_GNU
        {
            std::ifstream fin("/proc/loadavg", std::ios::in);
            double instant;
            double five_min;
            fin >> instant >> five_min;
            fin.close();
        }
        #endif
        
        /*std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();*/
        
        sleep(5);
        client->broadcastPing();
    }
    
    return 0;
}