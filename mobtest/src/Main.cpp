#include "Root.h"
#include "Kernel.h"
#include "GMalloc.h"

typedef mob::float4 float4;

int main(int argc, char* argv[])
{

    std::cout << "starting mobtest..." << std::endl;

    mob::root mob;
    mob.init(argc, argv);
    std::cout << "initialized" << std::endl;
    
    
    //TODO: allow indicating in/out for data
    //      also hepers for converting C-style arrays to gmem arrays
    mob::gmem<float4> v("v", mob, 400); 
    mob::gmem<float4> p("p", mob, 400);
    mob::gmem<float4> x("x", mob, 400);
    
    //Initialize particles
    float d = 2.0;
    size_t idx = 0;
    for(size_t i=0; i<5; i++){
        for(size_t j=0; j<8; j++){
            for(size_t k=0; k<10; k++){
                v.init(idx, float4(0.0f, 0.0f, 0.0f, 1.0f));
                p.init(idx, float4(d*float(i) - (5.0f/2.0),d*float(j), d*float(k) - (10.0f/2.0), 1.0));
                x.init(idx, float4(d*float(i) - (5.0f/2.0),d*float(j), d*float(k) - (10.0f/2.0), 1.0));
            }
        }
    }

    float dt = 1.0f/240.0f;
    
    mob::kernel integrate_forces("integrate_forces", [&v, &p, &x, dt](size_t global_index){
        
        v.set(global_index, v[global_index] + (float4(0.0f, -9.8f, 0.0f, 1.0f) * 2.0f * dt));
        //v[global_index].w = 1.0;
        
        p.set(global_index, p[global_index] + v[global_index] * dt);
        //std::cout << p[global_index].str() << std::endl;
        //p[global_index].w = 1.0;        

    });
    
    mob.add_kernel(integrate_forces);
    
    mob.run();

    
    std::cout << "killing mobtest..." << std::endl;
    mob.kill();

    return 0;
}