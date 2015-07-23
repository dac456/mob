#ifndef __KERNEL_H
#define __KERNEL_H

#include "MobCommon.h"

namespace mob
{
    
    typedef std::function<void(size_t)> mob_kernel_func;
    typedef std::function<void()> mob_kernel_finished;

    class MOBAPI kernel{
    private:
        std::string _name;
        mob_kernel_func _kernel;
        mob_kernel_finished _exit_kernel;
        
        bool _threads_need_update;
        bool _implicit_barrier;
        std::vector< std::vector<size_t> > _threads;
        
    public:
        kernel(std::string name, mob_kernel_func kernel, bool implicit_barrier = false);
        ~kernel();
        
        void set_exit_kernel(mob_kernel_finished kernel);
        
        std::string get_name();
        
    private:
        void _exec(root& mob_root);
        
        friend class root;
    };

}

#endif