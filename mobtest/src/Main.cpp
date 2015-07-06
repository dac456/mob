#include "Root.h"
#include "Task.h"
#include "GMalloc.h"

int main(int argc, char* argv[])
{

    mob::root mob;
    mob.mob_init(argc, argv);
    
    
    //TODO: specify types like float4 and allow indicating in/out for data
    //      also hepers for converting C-style arrays to gmem arrays
    mob::gmem<float> a(4);
    a.set(0, 1.0f);
    a.set(1, 1.0f);
    a.set(2, 1.0f);
    a.set(3, 1.0f);
    
    mob::gmem<float> b(4);
    b.set(0, 1.0f);
    b.set(1, 1.0f);
    b.set(2, 1.0f);
    b.set(3, 1.0f);    
    
    mob::gmem<float> c(4);
    
    mob::task test([&a, &b, &c](size_t global_index){
        
        c.set(global_index, a[global_index] + b[global_index]);
        std::cout << c[global_index] << std::endl;
        
    });
    
    test.exec(mob);
    
    for(;;);
    
    //mob::gfree(diff);
    
    mob.mob_kill();

    return 0;
}