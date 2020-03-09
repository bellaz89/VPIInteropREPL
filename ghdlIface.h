#pragma once

#include<cstdio>
#include<cstring>
#include<cstdint>
#include<string>
#include<memory>
#include<bitset>
#include<iomanip>
#include<vector>
#include<sstream>
#include<functional>
#include<boost/fiber/all.hpp>
#include<vpi_user.h>

#include "vpiFnTable.h"

enum class WaveFormat {
    None  = 0,
    Vcd   = 1,
    Vcdgz = 2,
    Fst   = 3,
    Ghw   = 4
};


enum class SimuStatus {
    None,
    Ready,
    Error,
    Execute,
    Terminated
};

class HdlSimulation {

    public:
        HdlSimulation(std::string& simulationPath_,
                WaveFormat& waveFormat_,
                std::string& wavePath_);

        vpiHandle getHandle(std::string handleName);

        uint32_t getInt(vpiHandle);
        void setInt(vpiHandle, uint32_t value);

        uint64_t getLong(vpiHandle);
        void setLong(vpiHandle, uint64_t value);

        std::vector<uint8_t> getBigInt(vpiHandle);
        void setBigInt(vpiHandle, std::vector<uint8_t> &value);

        void eval();
        void sleep(uint64_t cycles);
        void end();
        void enableWave(){};
        void disableWave(){};
        bool isBufferedWrite(){ return false; };
        size_t printSignals();

        virtual ~HdlSimulation();

    private:

        VpiFnTable vpi;

        void registerCb(PLI_INT32 (*cb_t)(p_cb_data),
                PLI_INT32 reason,
                int64_t time);
        PLI_INT32 beginCb(p_cb_data data);
        PLI_INT32 endCb(p_cb_data data);

        PLI_INT32 delayRWCb(p_cb_data data);
        PLI_INT32 delayROCb(p_cb_data data);
        PLI_INT32 readWriteCb(p_cb_data data);
        PLI_INT32 readOnlyCb(p_cb_data data);

        void printNetInModule(vpiHandle module_handle);
        void entryPointCbRegister(VpiFnTable);
        void checkReady();
        void checkExecute();

        void checkError();

        SimuStatus status;

        std::string simulationPath;
        WaveFormat waveFormat;
        std::string wavePath;

        std::string errorStr;

        boost::fibers::fiber ghdlFiber;
};
