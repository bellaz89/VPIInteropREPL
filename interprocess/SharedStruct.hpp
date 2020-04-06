#pragma once
#include<boost/interprocess/managed_shared_memory.hpp>
#include<boost/interprocess/allocators/allocator.hpp>
#include<boost/interprocess/sync/interprocess_condition.hpp>
#include<boost/interprocess/containers/vector.hpp>
#include<cstdint>

using namespace boost::interprocess;

typedef allocator<uint8_t, managed_shared_memory::segment_manager>  ShmemAllocator;
typedef vector<uint8_t, ShmemAllocator> SharedVector;

enum class ProcStatus {
    init,
    ready,
    print_signals,
    get_signal_handle,
    read,
    write,
    sleep,
    close,
    error
};

class SharedStruct {
    public:
    SharedStruct(managed_shared_memory& segment) : 
        alloc_inst(segment.get_segment_manager()), 
        sleep_cycles(0), 
        proc_status(ProcStatus::init), 
        mutex(),
        cond_new_command(),
        cond_command_rsp(),
        closed(false),
        data(alloc_inst) {

        }

    virtual ~SharedStruct(){}

    const ShmemAllocator alloc_inst;
    uint64_t sleep_cycles;
    ProcStatus proc_status;
    interprocess_mutex mutex;
    interprocess_condition  cond_new_command;
    interprocess_condition  cond_command_rsp;
    size_t handle;
    bool closed;
    SharedVector data;
};
