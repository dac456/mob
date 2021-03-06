#include "Root.h"
#include "Kernel.h"
#include "GMalloc.h"

typedef mob::float4 float4;

/*struct quad_tree{
    quad_tree* nw;
    quad_tree* ne;
    quad_tree* sw;
    quad_tree* se;
    
    std::vector<size_t> p;
    
    float x;
    float z;
    float width;
    float depth;  
    
    quad_tree(float _x, float _z, float w, float d) : nw(nullptr), ne(nullptr), sw(nullptr), se(nullptr){
        x = _x;
        z = _z;
        width = w;
        depth = d;
    }
    ~quad_tree(){
        if(nw) delete nw;
        if(ne) delete ne;
        if(sw) delete sw;
        if(se) delete se;
    }
    
    bool contains(float4 v)
        float half_w = (width/2.0f);
        float half_d = (depth/2.0f);
        
        if(v.x > (x - half_w) && v.x < (x + half_x) && v.z > (z - half_d) && v.z < (z + half_d)){
            return true;
        }
        else{
            return false;
        }
    }
    
    bool insert(size_t idx, float4 v){
        if(!contains(v)){
            return false;
        }
        
        if(p.size() < 4){
            p.push_back(idx);
            return true;
        }

        if(!ne && !nw && !se && !sw){
            subdivide();                             
        }
        
        if(nw->insert(idx, v)){
            return true;
        }
        
        if(ne->insert(idx, v)){
            return true;
        }        

        if(sw->insert(idx, v)){
            return true;
        }
        
        if(se->insert(idx, v)){
            return true;
        } 
        
        return false;

    }  
    
    void subdivide(){
        float half_w = width/2.0f;
        float half_d = depth/2.0f;
        float cx = x - half_w;
        float cz = z - half_d;
        
        nw = new quad_tree(cx, cz, half_w, half_d);
        
        cx = x + half_w; cz = y - half_d;
        ne = new quad_tree(cx, cz, half_w, half_d);
    }
};*/

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
    
    mob::gmem<float4> l("l", mob, 200);
    
    std::map<size_t, std::vector< std::pair<size_t, std::pair<float,float4> > >> local_group;
    
    //Initialize particles
    //quad_tree* tree;
    
    float d = 2.0f;
    size_t idx = 0;
    for(size_t i=0; i<2; i++){
        for(size_t j=0; j<10; j++){
            for(size_t k=0; k<10; k++){
                v.init(idx, float4(0.0f, 0.0f, 0.0f, 1.0f));
                p.init(idx, float4(d*float(i) - (2.0f/2.0f),d*float(j) + 0.0f, d*float(k) - (10.0f/2.0f), 1.0f));
                x.init(idx, float4(d*float(i) - (2.0f/2.0f),d*float(j) + 0.0f, d*float(k) - (10.0f/2.0f), 1.0f));
                
                c.init(idx, float4(0.0f, 0.0f, 0.0f, 0.0f));
                
                l.init(idx, float4(-1.0f, -1.0f, -1.0f, -1.0f));
                
                idx++;
            }
        }
    }

    float dt = 1.0f/240.0f;
    
    mob::kernel integrate_forces("integrate_forces", [&v, &p, &x, dt](size_t global_index, size_t local_index){

        v.set(global_index, v[global_index] + (float4(0.0f, -9.8f, 0.0f, 1.0f) * 2.0f * dt));
        v.set(global_index, float4(v[global_index].x,v[global_index].y,v[global_index].z, 1.0f));
        
        p.set(global_index, p[global_index] + (v[global_index] * dt));
        p.set(global_index, float4(p[global_index].x,p[global_index].y,p[global_index].z, 1.0f));

    }, true);
    
    boost::barrier bar1(4);
    boost::barrier bar2(4);
    float4 local_block[4];
    
    
    
    mob::kernel project_particles("project_particles", [&p, &c, &l, &local_block, &local_group, &bar1, &bar2](size_t global_index, size_t local_index){

        for(size_t k=0; k<4; k++){
            size_t nb = 200/4;
            
            size_t nc = 0;
            float4 d_avg(0.0f, 0.0f, 0.0f, 0.0f);
            
            //for(int j=0; j<nb; j++){
                //std::cout << (j*4)+local_index << std::endl;
            //    local_block[local_index] = p[(j*4)+local_index];
            //    bar1.wait();
            

                if(l[global_index].x != -1.0f && l[global_index].y != -1 && l[global_index].z != -1 && l[global_index].w != -1){
                    for(size_t i=0; i<4; i++){
                        if(size_t(l[global_index][i]) != global_index){
                                
                                //float4 delta = local_block[i] - p[global_index];
                                float4 delta = p[size_t(l[global_index][i])] - p[global_index];
                                float delta_length = delta.length();
                                
                                //if(!isnan(delta_length)){
                                    float C = delta_length - 2.0f;
                                    if(C < 0.0f){
                                        float s = 2.0f;
                                        
                                        d_avg = d_avg + ((delta/delta_length)*s*C * (1.0f - (1.0f - pow(0.9f, k))));
                                        d_avg.w = 1.0f;
                                        nc++;
                                                                    
                                    }
                                //}
                        }
                    }
                }
                else{
                    for(size_t i=0; i<200/*4*/; i++){
                        if(i != global_index){
                                
                                //float4 delta = local_block[i] - p[global_index];
                                float4 delta = p[i] - p[global_index];
                                float delta_length = delta.length();
                                
                                if(!isnan(delta_length)){
                                    float C = delta_length - 2.0f;
                                    if(C < 0.0f){
                                        float s = 2.0f;
                                        
                                        d_avg = d_avg + ((delta/delta_length)*s*C * (1.0f - (1.0f - pow(0.9f, k))));
                                        d_avg.w = 1.0f;
                                        nc++;
                                                                    
                                    }
                                }
                        }
                    }
                }
                
            //    bar2.wait();
            //}
            
            if(c[global_index].w != 0.0f){
                float4 delta = p[global_index] - c[global_index];
                delta.w = 1.0f;
                
                float delta_length = delta.length();
                
                if(!isnan(delta_length)){
                    float C = delta.dot(float4(0.0f, 1.0f, 0.0f, 1.0f));
                    
                    if(C <= 0.0f){
                        p.set(global_index, p[global_index] + (0.5f*C*C));
                    }
                }
                
                //c.set(global_index, float4(0.0f, 0.0f, 0.0f, 0.0f));
            }
            
            if(nc == 0){
                nc = 1;
            }
            //if(d_avg.str().find("nan") == std::string::npos){
                p.set(global_index, p[global_index] + (d_avg/float(nc)));
                p.set(global_index, float4(p[global_index].x,p[global_index].y,p[global_index].z, 1.0f));
            //}
        }
        
    }, true);
    
    mob::kernel correct_system("correct_system", [&v, &p, &x, &c, &l, dt](size_t global_index, size_t local_index){
        
        std::cout << l[global_index].str() << std::endl;
        
        v.set(global_index, (p[global_index] - x[global_index])/dt);
        v.set(global_index, float4(v[global_index].x,v[global_index].y,v[global_index].z, 1.0f));
        
        x.set(global_index, p[global_index]);
        x.set(global_index, float4(x[global_index].x,x[global_index].y,x[global_index].z, 1.0f));
        
        if(c[global_index].w != 0.0f){
            float4 n = float4(0.0f, 1.0f, 0.0f, 1.0f);
            
            float4 vn = n * n.dot(v[global_index]);
            float4 vt = v[global_index] - vn;
            
            v.set(global_index, (vn*-0.05f) + (vt*0.02f));
            
            c.set(global_index, float4(0.0f, 0.0f, 0.0f, 0.0f));
        }
        
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