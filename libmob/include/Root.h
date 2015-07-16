#ifndef __ROOT_H
#define __ROOT_H

#include "MobCommon.h"

namespace mob
{
    
    struct node_task_data{
        std::string prgm_name;
        std::vector<size_t> task_list;
        
        template<typename Archive>
        void serialize(Archive& ar, const unsigned int version){
            ar & prgm_name;
            ar & task_list;
        }
    };
    
    struct prgm_started_data{
        std::string node_name;
        std::string prgm_name;
        bool status;
        
        template<typename Archive>
        void serialize(Archive& ar, const unsigned int version){
            ar & node_name;
            ar & prgm_name;
            ar & status;
        }
    };       

    class /*MOBAPI*/ root{
    private:
        //ASIO
        asio::io_service _service;
        asio::ip::udp::socket _socket;
        asio::ip::udp::endpoint _sender_endpoint;
        char _buffer[512];
                
        //mob
        std::string _prgm_name;
        bool _waiting_for_tasks;
        
        std::map<std::string, kernel*> _kernel_map;
        
        boost::signals2::signal<void(size_t)> _sig_remote_get;
        
    public:
        root();
        ~root();
        
        void mob_init(int argc, char* argv[]);
        void mob_kill();
        
        void add_kernel(kernel& kern); //TODO: figure out why this hangs without '&'
        void exec_kernel(std::string name);
        
        const std::string get_name() const;
        bool is_waiting_for_tasks();
        
        TaskList* get_task_list();
        
    private:       
        void _start_accept();
        void _handle_receive(const boost::system::error_code& err, const size_t bytesReceived);
        
        //void _handle_node_ping_lib(node_message& msg);
        void _handle_prgm_get_mem(node_message& msg);
        
        void _prgm_send_mem(node_message& msg);
        void _prgm_get_mem(node_message& msg);
        //void _prgm_finished();
        
        typedef boost::signals2::signal<void(size_t)>::slot_type remote_get_event;
        boost::signals2::connection _connect_remote_get(const remote_get_event& e);
        
        friend class task;
        template<typename T> friend class gmem; //TODO: breaks export (I think)
    };

}

#endif