#ifndef __NODESERVER_H
#define __NODESERVER_H

#include "NodeCommon.h"
#include "NodeMessage.h"
#include "GMem.h"

namespace MobNode
{
    
    struct proc_info{
        proc::child* process;
        std::string name;
        size_t num_tasks;
        std::vector<size_t> task_indices; //indices assigned to this node
        
        bool started;
        bool finished;
        std::map<std::string, bool> running_on_node;
    };
    
    struct prgm_data{
        std::string prgm_name;
        size_t num_tasks;
        
        template<typename Archive>
        void serialize(Archive& ar, const unsigned int version){
            ar & prgm_name;
            ar & num_tasks;
        }        
    };
    
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

    class NodeServer{
    private:
        //ASIO
        asio::io_service* _service;
        asio::ip::udp::socket _socket;
        asio::ip::udp::endpoint _senderEndpoint;
        char _buffer[1024];
        
        //Node
        std::map<std::string, bool> _nodeMap;
        std::map<std::string, proc_info> _programMap;
        
    public:
        NodeServer(asio::io_service& service);
        ~NodeServer();
        
    private:
        void _launchProcess(std::string path, std::vector<std::string> args, proc::context ctx);
        
        void _startAccept();
        void _handleReceive(const boost::system::error_code& err, const size_t bytesReceived);
        void _handleSend(const boost::system::error_code& err, const size_t bytesSent);
        
        void _handleMsgPing(mob::node_message& msg);
        void _handleMsgPingLib(mob::node_message& msg);
        void _handleMsgPrgmSetMem(mob::node_message& msg);
        void _handleMsgPrgmGetMem(mob::node_message& msg);
        void _handleMsgLaunchPrgm(mob::node_message& msg);
        void _handleMsgStartPrgm(mob::node_message& msg);
        void _handleMsgSetTasks(mob::node_message& msg);
        void _handleMsgPrgmStarted(mob::node_message &msg);
    };

}

#endif