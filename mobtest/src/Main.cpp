#include "Root.h"
#include "Task.h"
#include "GMalloc.h"

int main(int argc, char* argv[])
{

    std::cout << "starting mobtest..." << std::endl;

    mob::root mob;
    mob.mob_init(argc, argv);
    std::cout << "initialized" << std::endl;
    
    
    //TODO: specify types like float4 and allow indicating in/out for data
    //      also hepers for converting C-style arrays to gmem arrays
    mob::gmem<float> a("a", mob, 4);
    a.set(0, 0.0f);
    a.set(1, 1.0f);
    a.set(2, 2.0f);
    a.set(3, 3.0f);
    
    mob::gmem<float> b("b", mob, 4);
    b.set(0, 0.0f);
    b.set(1, 1.0f);
    b.set(2, 2.0f);
    b.set(3, 3.0f);    
    
    mob::gmem<float> c("c", mob, 4);
    
    mob::task test([&a, &b, &c](size_t global_index){
        
        c.set(global_index, a[global_index] + b[global_index]);
        std::cout << c[global_index] << std::endl;        
        
    });
    
    test.exec(mob);
    
    std::cout << "enter 'q' to quit... " << std::endl;
    for(;;){
        if(std::cin.get() == 'q'){
            break;
        }
    }
    
    //mob::gfree(diff);
    
    std::cout << "killing mobtest..." << std::endl;
    mob.mob_kill();

    return 0;
}