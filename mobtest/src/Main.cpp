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
    mob::gmem<float4> v("v", mob, 8000); 
    mob::gmem<float4> p("p", mob, 8000);
    mob::gmem<float4> x("x", mob, 8000);
    
    //Initialize particles
    float d = 2.0f;
    size_t idx = 0;
    for(size_t i=0; i<20; i++){
        for(size_t j=0; j<20; j++){
            for(size_t k=0; k<20; k++){
                v.init(idx, float4(0.0f, 0.0f, 0.0f, 1.0f));
                p.init(idx, float4(d*float(i) - (5.0f/2.0f),d*float(j), d*float(k) - (10.0f/2.0f), 1.0f));
                x.init(idx, float4(d*float(i) - (5.0f/2.0f),d*float(j), d*float(k) - (10.0f/2.0f), 1.0f));
                
                idx++;
            }
        }
    }

    float dt = 1.0f/140.0f;
    
    mob::kernel integrate_forces("integrate_forces", [&v, &p, &x, dt](size_t global_index){
        
        v.set(global_index, v[global_index] + (float4(0.0f, -9.8f, 0.0f, 1.0f) * 2.0f * dt));
        v.set(global_index, float4(v[global_index].x, v[global_index].y, v[global_index].z, 1.0f));
        
        p.set(global_index, p[global_index] + v[global_index] * dt);
        p.set(global_index, float4(p[global_index].x, p[global_index].y, p[global_index].z, 1.0f));       

    });
    
    mob.add_kernel(integrate_forces);
    
    mob.run();

    
    std::cout << "killing mobtest..." << std::endl;
    mob.kill();

    return 0;
}