#include "Kernel.h"
#include "Root.h"

namespace mob
{
    
    kernel::kernel(std::string name, mob_kernel_func kernel){
        _name = name;
        _kernel = kernel;
    }
    
    kernel::~kernel(){
        
    }

    void kernel::_exec(root& mob_root){
        boost::thread exec_thread([&](){            

            bip::managed_shared_memory segment(bip::open_only, mob_root.get_name().c_str());      
            TaskList* mem = segment.find<TaskList>("task_list").first;            
            
            while(mem->empty());
            std::cout << "got " << mem->size() << " tasks" << std::endl;
            TaskList::const_iterator it;
            for(it = mem->cbegin(); it != mem->cend(); it++){
                std::cout << "loop" << std::endl;
                //boost::thread t(_task, idx);
                _kernel(*it);
            }
            
            //mob_root._prgm_finished();
        });
    }
    
    void kernel::set_exit_kernel(mob_kernel_finished kernel){
        _exit_kernel = kernel;
    }
    
    std::string kernel::get_name(){
        return _name;
    }

}