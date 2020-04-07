#include "SharedMemIface.hpp"
#include<iostream>
#include<ostream>
#include<bitset>
#include<iomanip>
#include<functional>
#include<vector>
#include<tuple>
#include<map>
#include<sstream>
#include<string>

typedef std::vector<std::string> str_vec_t;
typedef bool(*repl_cb_t)(SharedMemIface&, str_vec_t);
typedef std::tuple<repl_cb_t, std::string> cmd_map_t;

bool getInt(SharedMemIface&, std::vector<std::string>);
bool getLong(SharedMemIface&, std::vector<std::string>);
bool getBigInt(SharedMemIface&, std::vector<std::string>);
bool setInt(SharedMemIface&, std::vector<std::string>);
bool setLong(SharedMemIface&, std::vector<std::string>);
bool setBigInt(SharedMemIface&, std::vector<std::string>);
bool printSignals(SharedMemIface&, std::vector<std::string>);
bool sleep(SharedMemIface&, std::vector<std::string>);
bool eval(SharedMemIface&, std::vector<std::string>);
bool help(SharedMemIface&, std::vector<std::string>);
bool end(SharedMemIface&, std::vector<std::string>);

std::map<std::string, cmd_map_t> commands {
    {"getInt", {&getInt, "<signalName> get an int"}},
        {"getLong", {&getLong, "<signalName> get a long"}},
        {"getBigInt", {&getBigInt, "<signalName> get a big integer as binary"}},
        {"setInt", {&setInt, "<signalName> <integer> set a integer"}},
        {"setLong", {&setLong, "<signalName> <integer> set a long"}},
        {"setBigInt", {&setBigInt, "<signalName> <binary> set a big integer"}},
        {"printSignals", {&printSignals, "print signal names of the top module"}},
        {"sleep", {&sleep, "<integer> sleep for a number of steps"}},
        {"eval", {&eval, "evaluate the module"}},
        {"help", {&help, "this command"}},
        {"end", {&end, "stop the simulation and exit"}}
};

bool getInt(SharedMemIface &simu, std::vector<std::string> cmdStr) {

    uint64_t handle = simu.get_signal_handle(cmdStr[1]);

    if (handle == 0){

        std::cout << " signal " << cmdStr[1] << " not found" << std::endl;
        return false;
    };

    std::cout << " = " << simu.read_u32(handle) << std::endl;

    return false;
}

bool getLong(SharedMemIface &simu, std::vector<std::string> cmdStr) {

    uint64_t handle = simu.get_signal_handle(cmdStr[1]);

    if (handle == 0){

        std::cout << " signal " << cmdStr[1] << " not found" << std::endl;
        return false;
    };

    std::cout << " = " << simu.read_u64(handle) << std::endl;

    return false;
}


bool getBigInt(SharedMemIface &simu, std::vector<std::string> cmdStr) {

    uint64_t handle = simu.get_signal_handle(cmdStr[1]);

    if (handle == 0){

        std::cout << " signal " << cmdStr[1] << " not found" << std::endl;
        return false;
    };

    std::vector<uint8_t> ret = simu.read(handle);
    std::cout << " = ";
    for(auto& x: ret) std::cout << std::setw(8)  << std::setfill('0') << std::bitset<8>(x);
    std::cout << std::endl;

    return false;
}


bool setInt(SharedMemIface &simu, std::vector<std::string> cmdStr) {

    uint64_t handle = simu.get_signal_handle(cmdStr[1]);

    if (handle == 0){

        std::cout << " signal " << cmdStr[1] << " not found" << std::endl;
        return false;
    };

    simu.write_u32(handle, std::stol(cmdStr[2]));

    return false;
}

bool setLong(SharedMemIface &simu, std::vector<std::string> cmdStr) {

    uint64_t handle = simu.get_signal_handle(cmdStr[1]);

    if (handle == 0){

        std::cout << " signal " << cmdStr[1] << " not found" << std::endl;
        return false;
    };

    simu.write_u64(handle, std::stoi(cmdStr[2]));

    return false;
}

bool setBigInt(SharedMemIface &simu, std::vector<std::string> cmdStr) {

    uint64_t handle = simu.get_signal_handle(cmdStr[1]);

    if (handle == 0){

        std::cout << " signal " << cmdStr[1] << " not found" << std::endl;
        return false;
    };

    std::vector<uint8_t> val;

    while((cmdStr[2].size()%8) != 0) {
        cmdStr[2].insert(cmdStr[2].begin(), '0');
    }

    for(size_t i=0; i<(cmdStr[2].size()/8); i++){
        char accumVal[9];
        accumVal[8] = '\0';
        strncpy(accumVal, cmdStr[2].c_str()+i*8, 8);
        val.push_back(std::stoul(std::string(accumVal),
                    nullptr,
                    2));
    }

    simu.write(handle, val);

    return false;
}



bool printSignals(SharedMemIface &simu, std::vector<std::string> cmdStr) {

    (void) cmdStr;
    std::cout << std::endl;
    simu.print_signals();
    return false;
}

bool sleep(SharedMemIface &simu, std::vector<std::string> cmdStr) {

    simu.sleep(std::stoi(cmdStr[1]));
    return false;
}

bool eval(SharedMemIface &simu, std::vector<std::string> cmdStr) {

    (void) cmdStr;
    simu.eval();
    return false;
}

bool help(SharedMemIface &simu, std::vector<std::string> cmdStr) {

    (void) simu;
    (void) cmdStr;
    std::cout << std::endl << " Available commands:" << std::endl;
    std::map<std::string, cmd_map_t>::iterator it;
    for(it = commands.begin(); it != commands.end(); it++) {

        std::cout << "   " << it->first << " : ";
        std::cout << std::get<1>(it->second) << std::endl;
    }

    std::cout << std::endl;
    return false;
}

bool end(SharedMemIface &simu, std::vector<std::string> cmdStr) {

    (void) cmdStr;
    simu.close();
    return true;
}

int main(int argc, char **argv){

    std::string shared_name("default_shared");
    size_t shared_size = 65536;
    (void) argc;
    (void) argv;

    SharedMemIface simu(shared_name, shared_size);
    std::system(RUN_SIMULATOR_COMMAND);
    simu.eval();

    bool terminate = false;

    while (terminate == false){
        std::cout << "> ";
        std::string line;
        std::vector<std::string> cmdStr;
        std::getline(std::cin, line);
        std::istringstream iss(line);
        for(std::string s; iss >> s;) cmdStr.push_back(s);

        std::map<std::string, cmd_map_t>::iterator it = commands.find(cmdStr[0]);
        if(it == commands.end()) {

            std::cout << " command " << cmdStr[0] << " not found" << std::endl;
            std::cout << std::endl;
            help(simu, cmdStr);

        } else {

            terminate = std::get<0>(commands[cmdStr[0]])(simu, cmdStr);
        }
    }

    return 0;
}
