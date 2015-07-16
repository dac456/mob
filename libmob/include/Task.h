#ifndef __TASK_H
#define __TASK_H

#include "MobCommon.h"

namespace mob
{
    
    typedef std::function<void(size_t)> mob_task_func;
    typedef std::function<void()> mob_task_finished;

    class MOBAPI task{
    private:
        mob_task_func _task;
        mob_task_finished _exit_task;
        
    public:
        task(mob_task_func task);
        ~task();
        
        void exec(root& mob_root);
        void set_exit_task(mob_task_finished task);
    };

}

#endif