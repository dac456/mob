#include "Host.h"

int main(int argc, char* argv[])
{
    
    mob::host host;
    host.launch("mobtest", "test");
    
    std::vector<float> result = host.capture("mobtest", "c");
    
    /*for(size_t i=0; i<result.size(); i++){
        std::cout << result[i] << std::endl;
    }*/
    for(;;);
    
    return 0;

}