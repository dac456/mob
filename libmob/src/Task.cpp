#include "Task.h"
#include "Root.h"

namespace mob
{
    
    task::task(mob_task_func task){
        _task = task;
    }
    
    task::~task(){
        
    }

    void task::exec(root& mob_root){
        boost::thread exec_thread([&](){            
            bip::managed_shared_memory segment(bip::open_only, mob_root.get_name().c_str());      
            TaskList* mem = segment.find<TaskList>("task_list").first;            
            
            while(mem->empty());
            std::cout << "got " << mem->size() << " tasks" << std::endl;
            TaskList::const_iterator it;
            for(it = mem->cbegin(); it != mem->cend(); it++){
                //boost::thread t(_task, idx);
                _task(*it);
            }
            
            //mob_root._prgm_finished();
        });
    }
    
    void task::set_exit_task(mob_task_finished task){
        _exit_task = task;
    }

}