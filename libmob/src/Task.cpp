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
        for(auto idx : mob_root._task_indices){
           boost::thread t(_task, idx);
        }
    }

}