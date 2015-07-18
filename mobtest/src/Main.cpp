#include "Root.h"
#include "Kernel.h"
#include "GMalloc.h"

int main(int argc, char* argv[])
{

    std::cout << "starting mobtest..." << std::endl;

    mob::root mob;
    mob.mob_init(argc, argv);
    std::cout << "initialized" << std::endl;
    
    
    //TODO: specify types like float4 and allow indicating in/out for data
    //      also hepers for converting C-style arrays to gmem arrays
    mob::gmem<float> a("a", mob, 400);
    for(size_t i=0; i<400; i++){
        a.init(i, (float)i);
    }
    
    mob::gmem<float> b("b", mob, 400);
    for(size_t i=0; i<400; i++){
        b.init(i, (float)i);
    }    
    
    mob::gmem<float> c("c", mob, 400);
    
    mob::kernel test("test", [&a, &b, &c](size_t global_index){
        
        c.set(global_index, a[global_index] + b[global_index]);
        //std::cout << c[global_index] << std::endl;   
        /*std::ofstream fout;
        fout.open("/home/dcook/out.txt", std::ios::app);
        fout << c[global_index] << std::endl;
        fout.close(); */    
        
    });
    
    mob.add_kernel(test);
    //mob.exec_kernel("test");
    
    //std::cout << "enter 'q' to quit... " << std::endl;
    for(;;){
        /*for(size_t i=0; i<4; i++){
            std::cout << c[i] << std::endl;
        }
        if(std::cin.get() == 'q'){
            break;
        }*/
    }
    
    //mob::gfree(diff);
    
    std::cout << "killing mobtest..." << std::endl;
    mob.mob_kill();

    return 0;
}