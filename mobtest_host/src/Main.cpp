#include "Host.h"

#include <unistd.h>

int main(int argc, char* argv[])
{
    
    mob::host host;
    
    for(;;){
        host.launch("mobtest", "test");
        host.wait("mobtest", "test");
    
        std::vector<float> result = host.capture("mobtest", "c");
        
        for(size_t i=0; i<result.size(); i++){
            std::cout << result[i] << std::endl;
        }        
        
        sleep(1);
    }
    
    return 0;

}