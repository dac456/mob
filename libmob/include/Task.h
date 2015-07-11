#ifndef __TASK_H
#define __TASK_H

#include "MobCommon.h"

namespace mob
{
    
    typedef std::function<void(size_t)> mob_task_func;

    class MOBAPI task{
    private:
        mob_task_func _task;
        
    public:
        task(mob_task_func task);
        ~task();
        
        void exec(root& mob_root);
    };

}

#endif