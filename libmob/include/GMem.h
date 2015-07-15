#ifndef __GMEM_H
#define __GMEM_H

#include "MobCommon.h"
#include "NodeMessage.h"
#include "Root.h"

namespace mob
{

    struct set_mem{
    //private:
    //    friend class boost::serialization::access;
        
        std::string node_name;
        std::string prgm_name;
        std::string var_name;
        
        size_t idx;
        
        std::string val_type;
        std::string val;
        
        template<typename Archive>
        void serialize(Archive& ar, const unsigned int version){
            ar & node_name;
            ar & prgm_name;
            ar & var_name;
            ar & idx;
            ar & val_type;
            ar & val;
        }
    };
    
    template<typename T> 
    class gmem{
    private:
        const size_t _sz;
        
        std::string _name;
        std::string _mem_name;
        root* _mob_root;
        
        std::pair<size_t, bool> _waiting_for_remote;
        boost::signals2::connection _remote_get;
        
    public:
        gmem(std::string name, root& mob_root, const size_t sz) : _sz(sz){
            _name = name;
            
            //Create shared region
            bip::managed_shared_memory segment(bip::open_only, mob_root.get_name().c_str());
            segment.find_or_construct<T>(name.c_str())[sz](0);
            
            _mob_root = &mob_root;

            _remote_get = _mob_root->_connect_remote_get(boost::bind(&gmem::remote_get_notify, this, _1));
        }
        
        ~gmem(){
            bip::shared_memory_object::remove(_name.c_str());
        }
        
        void set(size_t idx, T val){
            //TODO: update local (or remote) node with new value (?)
            //      need some way of tracking which node is responsible for which tasks
            //      -> broadcast from each node each time task assignment is updated?
            
            //Update local memory
            bip::managed_shared_memory segment(bip::open_only, _mob_root->get_name().c_str());
            std::pair<T*, bip::managed_shared_memory::size_type> res;
            
            res = segment.find<T>(_name.c_str());
            T* mem = res.first;

            (*(mem+idx)) = val;

            //Update remote if not our task
            if(std::find(_mob_root->_task_indices.begin(), _mob_root->_task_indices.end(), idx) == _mob_root->_task_indices.end()){
                _send_mem(_name, val, idx);
            }
        }
        
        size_t size(){
            return _sz;
        }
        
        std::string name(){
            return _name;
        }
        
        const T operator[] (const int idx){
            assert(idx >= 0 && idx < _sz);
            
            if(std::find(_mob_root->_task_indices.begin(), _mob_root->_task_indices.end(), idx) != _mob_root->_task_indices.end()){
                bip::managed_shared_memory segment(bip::open_only, _mob_root->get_name().c_str());
                std::pair<T*, bip::managed_shared_memory::size_type> res;
                
                res = segment.find<T>(_name.c_str());
                T* mem = res.first;
                
                return (*(mem+idx));
            }
            else{
                _waiting_for_remote = std::make_pair(idx, true);
                
                return _get_mem(_name, idx);
            }
        }
        
        void remote_get_notify(size_t task_idx){
            //If this is the droid we're searching for...
            if(_waiting_for_remote.first == task_idx){
                _waiting_for_remote.second = false;
            }
        }
        
    private:
        void _send_mem(std::string var_name, T val, size_t task_idx){
            node_message msg(PRGM_SET_MEM);
            
            set_mem msg_data;
            msg_data.prgm_name = _mob_root->get_name();
            msg_data.var_name = var_name;
            msg_data.idx = task_idx;
            msg_data.val_type = std::string(typeid(val).name());
            
            std::stringstream val_str;
            val_str << val;
            msg_data.val = val_str.str();
                
            std::stringstream msg_stream;
            boost::archive::text_oarchive oa(msg_stream);
            oa << msg_data;
            
            msg.set_data(msg_stream.str().c_str(), msg_stream.str().size());
            
            _mob_root->_prgm_send_mem(msg);

        }
        
        const T _get_mem(std::string var_name, size_t task_idx) const{
            node_message msg(PRGM_GET_MEM);
            
            set_mem msg_data;
            msg_data.node_name = asio::ip::host_name();
            msg_data.prgm_name = _mob_root->get_name();
            msg_data.var_name = var_name;
            msg_data.idx = task_idx;
            msg_data.val_type = std::string(typeid(T).name());
              
            std::stringstream msg_stream;
            boost::archive::text_oarchive oa(msg_stream);
            oa << msg_data;
            
            msg.set_data(msg_stream.str().c_str(), msg_stream.str().size());
            
            _mob_root->_prgm_get_mem(msg);
            while(_waiting_for_remote.second); //TODO: timeout and detect bad node
            
            bip::managed_shared_memory segment(bip::open_only, _mob_root->get_name().c_str());
            std::pair<T*, bip::managed_shared_memory::size_type> res;
            
            res = segment.find<T>(_name.c_str());
            T* mem = res.first;
            
            return (*(mem+task_idx));                          
        }
        
    };
    
}

#endif