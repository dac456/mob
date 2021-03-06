#ifndef __ROOT_H
#define __ROOT_H

#include "MobCommon.h"

namespace mob
{
    
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
        std::vector<char> _buffer;
                
        //mob
        std::string _prgm_name;
        volatile bool _waiting_for_tasks;
        
        std::map<std::string, kernel*> _kernel_map;
        std::map<std::string, void*> _allocated_mem;
        
        boost::signals2::signal<void(std::string,size_t,std::string)> _sig_remote_get;
        boost::signals2::signal<void(size_t)> _sig_remote_move;
        
        std::mutex _shared_mem_write;
        std::mutex _host_get_mem_mtx;
        std::stack<node_message*> _mem_send_queue;
        
    public:
        root();
        ~root();
        
        void init(int argc, char* argv[]);
        void kill();
        void run();
        
        void add_kernel(kernel& kern); //TODO: figure out why this hangs without '&'
        void exec_kernel(std::string name);
        
        const std::string get_name() const;
        bool is_waiting_for_tasks();
        
        TaskList* get_task_list();
        
    private:       
        void _start_accept();
        void _handle_receive(const boost::system::error_code& err, const size_t bytesReceived);
        void _handle_send(const boost::system::error_code& err, const size_t bytesSent);
        
        //void _handle_node_ping_lib(node_message& msg);
        void _handle_prgm_get_mem(node_message& msg);
        void _handle_prgm_move_tasks(node_message& msg);
        void _handle_host_exec_kernel(node_message& msg);
        void _handle_host_get_mem(node_message& msg);
        void _handle_host_set_mem(node_message& msg);
        
        void _host_get_mem(prgm_var_data data, size_t sz);
        void _prgm_send_mem(node_message& msg);
        void _handle_mem_send(const boost::system::error_code& err, const size_t bytesSent);
        void _prgm_get_mem(node_message& msg);
        void _prgm_mov_task(std::string to_node, size_t idx);
        void _kernel_started(std::string kernel);
        void _kernel_finished(std::string kernel);
        
        typedef boost::signals2::signal<void(std::string,size_t,std::string)>::slot_type remote_get_event;
        typedef boost::signals2::signal<void(size_t)>::slot_type remote_move_event;
        boost::signals2::connection _connect_remote_get(const remote_get_event& e);
        boost::signals2::connection _connect_remote_move(const remote_move_event& e);
        
        friend class kernel;
        template<typename T> friend class gmem; //TODO: breaks export (I think)
    };

}

#endif