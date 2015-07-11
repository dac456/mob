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
        
        std::string prgm_name;
        std::string var_name;
        
        size_t idx;
        
        std::string val_type;
        std::string val;
        
        template<typename Archive>
        void serialize(Archive& ar, const unsigned int version){
            //ar & prgm_name_length;
            ar & prgm_name;
            //ar & var_name_length;
            ar & var_name;
            ar & idx;
            ar & val_type;
            ar & val;
        }
    };
    
    template<typename T> 
    class gmem{
    private:
        //T* _data;
        const size_t _sz;
        
        std::string _name;
        std::string _mem_name;
        root* _mob_root;
        
    public:
        gmem(std::string name, root& mob_root, const size_t sz) : _sz(sz){
            _name = name;
            
            //Create shared region
            bip::managed_shared_memory segment(bip::open_only, mob_root.get_name().c_str());
            segment.construct<T>(name.c_str())[sz](0);
            
            //_data = new T[sz];
            _mob_root = &mob_root;
        }
        
        ~gmem(){
            //delete[] _data;
            bip::shared_memory_object::remove(_mem_name.c_str());
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
        
        const T operator[] (const int idx) const{
            assert(idx >= 0 && idx < _sz);
            
            if(std::find(_mob_root->_task_indices.begin(), _mob_root->_task_indices.end(), idx) != _mob_root->_task_indices.end()){
                bip::managed_shared_memory segment(bip::open_only, _mob_root->get_name().c_str());
                std::pair<T*, bip::managed_shared_memory::size_type> res;
                
                res = segment.find<T>(_name.c_str());
                T* mem = res.first;
                
                return (*(mem+idx));
            }
            else{
                return _get_mem(_name, idx);
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
        
        T _get_mem(std::string var_name, size_t task_idx){
            
        }
        
    };
    
}

#endif