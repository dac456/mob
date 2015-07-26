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
    //      also helpers for converting C-style arrays to gmem arrays
    mob::gmem<float4> v("v", mob, 200); 
    mob::gmem<float4> p("p", mob, 200);
    mob::gmem<float4> x("x", mob, 200);
    mob::gmem<float4> c("c", mob, 200);
    
    //Initialize particles
    float d = 2.0f;
    size_t idx = 0;
    for(size_t i=0; i<2; i++){
        for(size_t j=0; j<10; j++){
            for(size_t k=0; k<10; k++){
                v.init(idx, float4(0.0f, 0.0f, 0.0f, 1.0f));
                p.init(idx, float4(d*float(i) - (2.0f/2.0f),d*float(j), d*float(k) - (10.0f/2.0f), 1.0f));
                x.init(idx, float4(d*float(i) - (2.0f/2.0f),d*float(j), d*float(k) - (10.0f/2.0f), 1.0f));
                
                c.init(idx, float4(0.0f, 0.0f, 0.0f, 0.0f));
                
                idx++;
            }
        }
    }

    float dt = 1.0f/60.0f;
    
    mob::kernel integrate_forces("integrate_forces", [&v, &p, &x, dt](size_t global_index){
        
        v.set(global_index, v[global_index] + (float4(0.0f, -9.8f, 0.0f, 1.0f) * 2.0f * dt));
        
        p.set(global_index, p[global_index] + (v[global_index] * dt));

    }, true);
    
    mob::kernel project_particles("project_particles", [&p, &c](size_t global_index){
        
        for(size_t k=0; k<5; k++){
            size_t nc = 0;
            float4 d_avg(0.0f, 0.0f, 0.0f, 0.0f);
            
            for(size_t i=0; i<200; i++){
                float4 delta = p[i] - p[global_index];
                float delta_length = delta.length();
                
                if(!isnan(delta_length)){
                    float C = delta_length - 2.0f;
                    if(C <= 0.0f){
                        float s = 0.5f;
                        
                        d_avg = d_avg + ((delta/delta_length)*s*C * (1.0f - (1.0f - pow(0.9, k))));
                        d_avg.w = 1.0f;
                        nc++;
                    }
                }
            }
            
            if(c[global_index].w != 0.0f){
                float4 delta = p[global_index] - c[global_index];
                float delta_length = delta.length();
                
                if(!isnan(delta_length)){
                    float C = delta.dot(float4(0.0f, 1.0f, 0.0f, 1.0f));
                    
                    if(C <= 0.0f){
                        p.set(global_index, p[global_index] + (0.5f*C*C));
                    }
                }
                
                c.set(global_index, float4(0.0f, 0.0f, 0.0f, 0.0f));
            }
            
            if(nc == 0){
                nc = 1;
            }
            if(d_avg.str().find("nan") == std::string::npos){
                p.set(global_index, p[global_index] + (d_avg/float(nc)));
            }
        }
        
    }, true);
    
    mob::kernel correct_system("correct_system", [&v, &p, &x, &c, dt](size_t global_index){
        
        v.set(global_index, (p[global_index] - x[global_index])/dt);
        x.set(global_index, p[global_index]);
        
        if((x[global_index].y-1.0f) < 0.0f){
            c.set(global_index, float4(x[global_index].x, 0.0f, x[global_index].z, 1.0f));
        }
        
    }, true);
    
    mob.add_kernel(integrate_forces);
    mob.add_kernel(project_particles);
    mob.add_kernel(correct_system);
    
    mob.run();

    
    std::cout << "killing mobtest..." << std::endl;
    mob.kill();

    return 0;
}