#ifndef __GMEM_H
#define __GMEM_H

#include "MobCommon.h"
#include "NodeMessage.h"
#include "Root.h"
#include "DataTypes.h"

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
        
        std::map<size_t, size_t> _ref_count;
        std::map<size_t, std::pair<size_t,bool>> _miss_map;
        
        std::pair<size_t, volatile bool> _waiting_for_remote;
        boost::signals2::connection _remote_get;
        boost::signals2::connection _remote_move;
        
        bool _dirty;
        
        std::mutex _gmem_set;
        std::mutex _gmem_get;
        
    public:
        gmem(std::string name, root& mob_root, const size_t sz) : _sz(sz){
            _name = name;
            _dirty = true;
            
            //Create shared region
            bip::managed_shared_memory segment(bip::open_only, mob_root.get_name().c_str());
            segment.find_or_construct<T>(name.c_str())[sz]();
            
            _mob_root = &mob_root;
            _mob_root->_allocated_mem[name] = this;

            //Connect signal for remote get
            _remote_get = _mob_root->_connect_remote_get(boost::bind(&gmem::remote_get_notify, this, _1, _2, _3));
            _remote_move = _mob_root->_connect_remote_move(boost::bind(&gmem::remote_move_notify, this, _1));
            
            //Initialize ref counts
            for(size_t i=0; i<sz; i++){
                _ref_count[i] = 0;
            }
        }
        
        ~gmem(){
            bip::shared_memory_object::remove(_name.c_str());
        }
        
        void init(size_t idx, T val){
            //Update local memory
            bip::managed_shared_memory segment(bip::open_only, _mob_root->get_name().c_str());
            std::pair<T*, bip::managed_shared_memory::size_type> res;
            
            res = segment.find<T>(_name.c_str());
            T* mem = res.first;

            (*(mem+idx)) = val;   
        }
        
        void set(size_t idx, T val){
            _gmem_set.lock();

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
            TaskList* task_list = segment.find<TaskList>("task_list").first; 
            
            
            //TODO: always send to make data capture easier. in practice should not do this
            if(std::find(task_list->begin(), task_list->end(), idx) == task_list->end()){
                _send_mem(_name, val, idx);
            }
            _gmem_set.unlock();             
        }
        
        void fetch(){
            //if(_dirty){
                /*bip::managed_shared_memory segment(bip::open_only, _mob_root->get_name().c_str());    
                TaskList* task_list = segment.find<TaskList>("task_list").first;  
                for(size_t i=0; i<_sz; i++){  
                    if(std::find(task_list->begin(), task_list->end(), i) == task_list->end()){
                        _waiting_for_remote = std::make_pair(i, true);
                        
                        auto x = _get_mem(_name, i);
                    }
                }*/
                
            //    _dirty = false;
            //}
        }
        
        size_t size(){
            return _sz;
        }
        
        std::string name(){
            return _name;
        }
        
        bool dirty(){
            return _dirty;
        }
        
        const T operator[] (const int idx){
            //_gmem_get.lock();
            assert(idx >= 0 && idx < _sz);
            _ref_count[idx]++;
            
            try{
                bip::managed_shared_memory segment(bip::open_only, _mob_root->get_name().c_str());    
                TaskList* task_list = segment.find<TaskList>("task_list").first;  
                
                if(std::find(task_list->begin(), task_list->end(), idx) != task_list->end()){
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
            catch(bip::interprocess_exception& e){
                std::cout << e.what() << std::endl;
            }          
            //_gmem_get.unlock();  
        }
        
        void remote_get_notify(std::string from_node, size_t task_idx, std::string var_name){
            //If this is the droid we're searching for...
            if(_waiting_for_remote.first == task_idx && var_name == _name && from_node != asio::ip::host_name()){
                _waiting_for_remote.second = false;
                _locality_miss(from_node, task_idx);
            }
        }
        
        void remote_move_notify(size_t task_idx){
            if(_miss_map.count(task_idx)){
                _miss_map.at(task_idx).second = false;
            }
        }
        
    private:
        void _set_data(float val, set_mem& data){
            data.val_type = "float";
            std::stringstream val_str;
            val_str << val;
            data.val = val_str.str();             
        }
        void _set_data(float4 val, set_mem& data){
            data.val_type = "float4";
            data.val = val.str();            
        }
        
        void _send_mem(std::string var_name, T val, size_t task_idx){
            node_message msg(PRGM_SET_MEM);
            
            set_mem msg_data;
            msg_data.prgm_name = _mob_root->get_name();
            msg_data.var_name = var_name;
            msg_data.idx = task_idx;

            _set_data(val, msg_data);
                
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
            
            if(typeid(T) == typeid(float)){
                msg_data.val_type = "float";
            }
            else if(typeid(T) == typeid(float4)){
                msg_data.val_type = "float4";
            }
            else{
                std::cout << "gmem: unknown typeid" << std::endl;
            }
              
            std::stringstream msg_stream;
            boost::archive::text_oarchive oa(msg_stream);
            oa << msg_data;
            
            msg.set_data(msg_stream.str().c_str(), msg_stream.str().size());
            
            _mob_root->_prgm_get_mem(msg);

            std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
            while(_waiting_for_remote.second){
                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
                if(millis > 10000){
                    break;
                }          
            }            
            
            bip::managed_shared_memory segment(bip::open_only, _mob_root->get_name().c_str());
            std::pair<T*, bip::managed_shared_memory::size_type> res;
            
            res = segment.find<T>(_name.c_str());
            T* mem = res.first;
            
            return (*(mem+task_idx));                          
        }
        
        void _locality_miss(std::string from_node, size_t idx){
            //std::cout << "_locality_miss " << idx << " " << from_node << std::endl;
            if(!_miss_map.count(idx)){
                _miss_map[idx] = std::make_pair(1, false);
            }
            else{
                _miss_map.at(idx).first++;
            }
            
            float miss_ratio = float(_miss_map.at(idx).first) / float(_ref_count[idx]);
            //std::cout << float(_miss_map.at(idx).first) << " " << float(_ref_count[idx]) << " " << miss_ratio << std::endl;
            /*if(miss_ratio > 0.5f && _ref_count[idx] > 5 && !_miss_map.at(idx).second){
                _mob_root->_prgm_mov_task(from_node, idx);
                _miss_map.at(idx).second = true;
            }*/
        }
        
        //friend class root;
        
    };
    
}

#endif