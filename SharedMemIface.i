 %module JNISharedMemIface
 %{
 #include "SharedMemIface.hpp"
 %}

%include "std_string.i"
%include "stdint.i"
%include "std_vector.i"

namespace std {
    %template(VectorUint8) vector<uint8_t>;
};

class SharedMemIface {
public:
    SharedMemIface(string shmem_name_, size_t shmem_size_);
    ~SharedMemIface();
    string print_signals();
    uint64_t get_signal_handle(string& handle_name);
    std::vector<uint8_t> read(uint64_t handle);
    uint64_t read_u64(uint64_t handle);
    uint32_t read_u32(uint64_t handle);
    void write(uint64_t handle, std::vector<uint8_t>& data);
    void write_u64(uint64_t handle, uint64_t data);
    void write_u32(uint64_t handle, uint32_t data);
    void sleep(uint64_t sleep_cycles);
    void eval();
    bool error_happened();
    string error_string();
    void close();
};

