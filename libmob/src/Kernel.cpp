#include "Kernel.h"
#include "Root.h"

namespace mob
{
    
    kernel::kernel(std::string name, mob_kernel_func kernel, bool implicit_barrier){
        _name = name;
        _kernel = kernel;
        _threads_need_update = true;
        _implicit_barrier = implicit_barrier;
        _threads.resize(8);
    }
    
    kernel::~kernel(){
        
    }

    void kernel::_exec(root& mob_root){
        boost::thread kernel_thread([&](){  
            
            mob_root._kernel_started(_name);          

            if(_threads_need_update){
                bip::managed_shared_memory segment(bip::open_only, mob_root.get_name().c_str());      
                TaskList* mem = segment.find<TaskList>("task_list").first;            
                
                while(mem->empty());
                
                std::cout << "got " << mem->size() << " tasks" << std::endl;
                
                size_t t = 0;
                for(size_t i=0; i<mem->size(); i++){
                    //std::cout << t << " " << (*mem)[i] << std::endl;
                    _threads[t].push_back((*mem)[i] );
                    t = (t + 1) % 8;
                }
                
                _threads_need_update = false;
            }
            
            boost::thread_group grp;
            for(size_t i=0; i<8; i++){
                //boost::thread exec_thread([=](){
                grp.create_thread([=](){
                    for(size_t j=0; j<_threads[i].size(); j++){
                        _kernel(_threads[i][j]);
                    }
                });
                //grp.add_thread(&exec_thread);
            }
            if(_implicit_barrier){ 
                grp.join_all();
            }           
            
            mob_root._kernel_finished(_name);
        });
    }
    
    void kernel::set_exit_kernel(mob_kernel_finished kernel){
        _exit_kernel = kernel;
    }
    
    std::string kernel::get_name(){
        return _name;
    }

}