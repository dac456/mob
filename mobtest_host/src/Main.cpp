#include "Host.h"

#include <unistd.h>

int main(int argc, char* argv[])
{
    
    mob::host host;
    
    for(;;){
        host.launch("mobtest", "integrate_forces");
        host.wait("mobtest", "integrate_forces");
    
        std::vector<mob::float4> result;
        result = host.capture_float4("mobtest", "p");
        
        for(size_t i=0; i<result.size(); i++){
            std::cout << result[i].str() << std::endl;
        }        
        
        sleep(1);
    }
    
    return 0;

}