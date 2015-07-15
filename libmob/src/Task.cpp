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
            TaskList* mem = segment.find_or_construct<TaskList>("task_list")(segment.get_segment_manager());
            
            std::cout << "starting exec" << std::endl;
            while(mem->empty());
            std::cout << "got " << mem->size() << " tasks" << std::endl;
            for(auto idx : *mem){
                //boost::thread t(_task, idx);
                _task(idx);
            }
        });
    }

}