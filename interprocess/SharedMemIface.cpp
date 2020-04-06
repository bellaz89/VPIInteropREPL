#include"SharedMemIface.hpp"
#include<boost/date_time/posix_time/posix_time.hpp> 
#include<boost/interprocess/sync/scoped_lock.hpp>
#include<boost/interprocess/managed_shared_memory.hpp>
#include<algorithm>
#include<string>
#include<vector>
#include<cassert>
#include<cstring>

using namespace boost::posix_time;
using namespace boost::gregorian;

SharedMemIface::SharedMemIface(string shared_mem_name_, size_t shared_mem_size_) :
    shared_mem_name(shared_mem_name_), shared_mem_size(shared_mem_size_) {
    shared_memory_object::remove(shared_mem_name.c_str());
    segment = managed_shared_memory(create_only, shared_mem_name.c_str(), shared_mem_size);
    shared_struct = segment.construct<SharedStruct>("SharedStruct")(segment);
    shared_struct->proc_status = ProcStatus::init;
}

string SharedMemIface::print_signals(){
    std::string ret;
    wait_ready();
    if(!error_happened()){
        shared_struct->proc_status = ProcStatus::print_signals;
        wait();
        assert(shared_struct->proc_status == ProcStatus::ready);
        ret = (const char*)shared_struct->data.data();
    }

    return ret;
}

uint64_t SharedMemIface::get_signal_handle(string& handle_name){
    wait_ready();
    if(!error_happened()){
        shared_struct->data.resize(handle_name.size());
        memcpy(shared_struct->data.data(), handle_name.c_str(), handle_name.size());
        shared_struct->proc_status = ProcStatus::get_signal_handle;
        wait();
        assert(shared_struct->proc_status == ProcStatus::ready);
        return shared_struct->handle;
    }

    return 0;
}


std::vector<uint8_t> SharedMemIface::read(uint64_t handle){
    std::vector<uint8_t> ret_vec;
    wait_ready();
    if(!error_happened()) {
        shared_struct->handle = handle;
        shared_struct->proc_status = ProcStatus::read;
        wait();
        assert(shared_struct->proc_status == ProcStatus::ready);
        ret_vec.resize(shared_struct->data.size());
        memcpy(ret_vec.data(), shared_struct->data.data(), shared_struct->data.size());
    }

    return ret_vec;
}

uint64_t SharedMemIface::read_u64(uint64_t handle){
    uint64_t ret = 0ul;
    std::vector<uint8_t> read_data = this->read(handle);
    size_t copy_size = std::min(8ul, read_data.size());
    size_t start_dest = 8ul-copy_size;
    size_t start_orig = read_data.size();
    if(start_orig < 8ul) start_orig = 0ul;
    else start_orig -= 8ul;
    memcpy(((uint8_t*)&ret)+start_dest, read_data.data()+start_orig, copy_size);
    return ret;
}

uint32_t SharedMemIface::read_u32(uint64_t handle){
    uint32_t ret = 0ul;
    std::vector<uint8_t> read_data = this->read(handle);
    size_t copy_size = std::min(4ul, read_data.size());
    size_t start_dest = 4ul-copy_size;
    size_t start_orig = read_data.size();
    if(start_orig < 4ul) start_orig = 0ul;
    else start_orig -= 4ul;
    memcpy(((uint8_t*)&ret)+start_dest, read_data.data()+start_orig, copy_size);
    return ret;
}

void SharedMemIface::write(uint64_t handle, std::vector<uint8_t>& data){
    wait_ready();
    if(!error_happened()) {
        shared_struct->handle = handle;
        shared_struct->data.resize(data.size());
        memcpy(shared_struct->data.data(), data.data(), data.size());
        shared_struct->proc_status = ProcStatus::write;
        wait();
        assert(shared_struct->proc_status == ProcStatus::ready);
    }
}

void SharedMemIface::write_u64(uint64_t handle, uint64_t data){
    
    std::vector<uint8_t> vdata;
    vdata.resize(8);
    *((uint64_t*) vdata.data()) = data;
    this->write(handle, vdata);
}

void SharedMemIface::write_u32(uint64_t handle, uint32_t data){
    std::vector<uint8_t> vdata;
    vdata.resize(4);
    *((uint32_t*) vdata.data()) = data;
    this->write(handle, vdata);
}

void SharedMemIface::sleep(uint64_t sleep_cycles){
    wait_ready();
    if(!error_happened()) {
        shared_struct->sleep_cycles = sleep_cycles;
        shared_struct->proc_status = ProcStatus::sleep;
        wait();
        assert(shared_struct->proc_status == ProcStatus::ready);
    }
}

void SharedMemIface::eval(){
    this->sleep(0);
}

void SharedMemIface::close(){
    wait_ready();
    if(!error_happened()) {
        shared_struct->proc_status = ProcStatus::close;
        scoped_lock<interprocess_mutex> lock(shared_struct->mutex); 
        assert(shared_struct->proc_status != ProcStatus::ready);
        shared_struct->cond_new_command.notify_one();
    }

    while(!shared_struct->closed);
}

bool SharedMemIface::error_happened(){
    return shared_struct->proc_status == ProcStatus::error;
}

std::string SharedMemIface::error_string(){
    
    return std::string((const char*)shared_struct->data.data());
}

SharedMemIface::~SharedMemIface(){
    shared_memory_object::remove(shared_mem_name.c_str());
}

bool SharedMemIface::wait(){

    scoped_lock<interprocess_mutex> lock(shared_struct->mutex); 

    assert(shared_struct->proc_status != ProcStatus::ready);
    shared_struct->cond_new_command.notify_one();
    while((shared_struct->proc_status != ProcStatus::ready) | 
          (shared_struct->proc_status != ProcStatus::error)){
        
        ptime local_time = microsec_clock::local_time();
        ptime end_wait = local_time + seconds(1);
        shared_struct->cond_command_rsp.timed_wait(lock, end_wait);
    }

    return error_happened();
}
