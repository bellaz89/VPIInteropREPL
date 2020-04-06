#include "ghdlIface.h"

using namespace std::placeholders;

HdlSimulation* simulation;

extern "C" int ghdl_main(int argc, const char **argv);

HdlSimulation::HdlSimulation(std::string& simulationPath_,
        WaveFormat& waveFormat_,
        std::string& wavePath_) :
    simulationPath(simulationPath_),
    waveFormat(waveFormat_),
    wavePath(wavePath_) {

        simulation = this; // horrible global state necessary to work with C callbacks
        this->status=SimuStatus::None;

        this->callerId = boost::this_fiber::get_id();

        this->ghdlFiber = boost::fibers::fiber([this](){
                std::vector<const char*> ghdlArguments;
                std::string waveArg;

                ghdlArguments.push_back("ghdl_main");

                switch(this->waveFormat){
                case WaveFormat::Vcd:
                waveArg = "--vcd=";
                waveArg += wavePath;
                ghdlArguments.push_back(&waveArg[0]);
                break;
                case WaveFormat::Vcdgz:
                waveArg = "--vcdgz=";
                waveArg += wavePath;
                ghdlArguments.push_back(&waveArg[0]);
                break;
                case WaveFormat::Fst:
                waveArg = "--fst=";
                waveArg += wavePath;
                ghdlArguments.push_back(&waveArg[0]);
                break;
                case WaveFormat::Ghw:
                waveArg = "--wave=";
                waveArg += wavePath;
                ghdlArguments.push_back(&waveArg[0]);
                break;
                default: break;
                }

                void (*entryPointCbAddr)(VpiFnTable) =  [](VpiFnTable table) {
                    simulation->entryPointCbRegister(table);
                };

                std::string entryPointPtrStr(" ENTRY_POINT_PTR=");
                entryPointPtrStr += std::to_string(reinterpret_cast<uint64_t>(entryPointCbAddr));
                std::string makeCmd("make vpi_plugin");
                makeCmd += entryPointPtrStr;
                printf("%s\n",makeCmd.c_str());
                if(system(makeCmd.c_str())){};

                ghdlArguments.push_back("--vpi=./vpi_plugin.vpi");
                ghdl_main(ghdlArguments.size(), &ghdlArguments[0]);
        });

        this->checkReady();
    }

vpiHandle HdlSimulation::getHandle(std::string handleName){

    this->checkReady();
    vpiHandle returnHandle;
    returnHandle = this->vpi.vpi_handle_by_name((PLI_BYTE8*)&handleName.c_str()[0],
            NULL);
    this->checkError();

    if(!returnHandle) {
        printf("vpi_handle_by_name failed with argument %s \n", handleName.c_str());
        return nullptr;
    }

    return returnHandle;
}

uint32_t HdlSimulation::getInt(vpiHandle handle){

    s_vpi_value valueStruct;
    this->checkReady();
    valueStruct.format = vpiBinStrVal;
    this->vpi.vpi_get_value(handle, &valueStruct);
    this->checkError();
    std::string binStr(valueStruct.value.str);
    if(binStr.size() > 32) binStr.erase(0, binStr.size()-32);

    return std::stoul(binStr,
            nullptr,
            2);
}

void HdlSimulation::setInt(vpiHandle handle, uint32_t value){

    s_vpi_value valueStruct;

    this->checkReady();
    valueStruct.format = vpiIntVal;
    valueStruct.value.integer = value;
    this->vpi.vpi_put_value(handle, &valueStruct, NULL, vpiNoDelay);
    this->checkError();
}

uint64_t HdlSimulation::getLong(vpiHandle handle){

    std::stringstream ss;

    this->checkReady();
    s_vpi_value valueStruct;
    valueStruct.format = vpiBinStrVal;
    this->vpi.vpi_get_value(handle, &valueStruct);
    this->checkError();
    std::string binStr(valueStruct.value.str);
    if(binStr.size() > 64) binStr.erase(0, binStr.size()-64);
    return std::stoull(binStr,
            nullptr,
            2);
}

void HdlSimulation::setLong(vpiHandle handle, uint64_t value){

    s_vpi_value valueStruct;
    std::stringstream ss;

    this->checkReady();
    ss << std::setw(64) << std::setfill('0') << std::bitset<64>(value);
    valueStruct.format = vpiBinStrVal;
    valueStruct.value.str = (PLI_BYTE8*)ss.str().c_str();
    this->vpi.vpi_put_value(handle, &valueStruct, NULL, vpiNoDelay);
    this->checkError();
}

std::vector<uint8_t> HdlSimulation::getBigInt(vpiHandle handle){

    std::vector<uint8_t> retval;
    s_vpi_value valueStruct;

    this->checkReady();
    valueStruct.format = vpiBinStrVal;
    this->vpi.vpi_get_value(handle, &valueStruct);
    this->checkError();
    size_t valueStrLen = strlen(valueStruct.value.str);
    size_t valueByteLen = valueStrLen/8;
    size_t bitShift = valueStrLen%8;

    if(bitShift != 0) {
        char accumStr[9] = "00000000";
        accumStr[8] = '\n';
        uint8_t accum = 0;
        strncpy(accumStr+(8-bitShift),
                valueStruct.value.str,
                bitShift);

        accum = std::stoul(std::string(accumStr),
                nullptr,
                2);
        retval.push_back(accum);
    }

    for(size_t i = 0; i<valueByteLen; i++){
        char accumStr[9];
        accumStr[8] = '\n';
        uint8_t accum = 0;
        strncpy(accumStr,
                valueStruct.value.str+bitShift+i*8,
                8);

        accum = std::stoul(std::string(accumStr),
                nullptr,
                2);

        retval.push_back(accum);
    }

    return retval;
}

void HdlSimulation::setBigInt(vpiHandle handle, std::vector<uint8_t> &value){

    s_vpi_value valueStruct;
    std::stringstream ss;

    this->checkReady();
    for (uint8_t& el : value) {
        ss << std::setw(8)
            << std::setfill('0')
            << std::bitset<8>(el);
    }

    valueStruct.format = vpiBinStrVal;
    valueStruct.value.str = (PLI_BYTE8*)ss.str().c_str();
    this->vpi.vpi_put_value(handle, &valueStruct, NULL, vpiNoDelay);
    this->checkError();
}

void HdlSimulation::eval() {
    this->sleep(0);
}

void HdlSimulation::sleep(uint64_t cycles) {

    this->checkReady();

    this->registerCb([](p_cb_data p_cb) -> PLI_INT32 {
            return simulation->delayROCb(p_cb);
            },
            cbAfterDelay,
            cycles);

    this->checkError();
    this->status = SimuStatus::Execute;
    this->checkReady();
}


void HdlSimulation::end() {

    this->checkReady();
    this->vpi.vpi_control(vpiFinish, 0);
    this->checkError();
    this->status = SimuStatus::Execute;
    this->ghdlFiber.join();
    this->checkError(true);
}

void HdlSimulation::registerCb(PLI_INT32(*f)(p_cb_data cbData),
        PLI_INT32 reason,
        int64_t time){

    s_cb_data cbData;
    s_vpi_time simuTime;

    if (time < 0) cbData.time = NULL;
    else
    {
        cbData.time = &simuTime;
        simuTime.type = vpiSimTime;
        simuTime.high = (PLI_INT32) (time >> 32);
        simuTime.low = (PLI_INT32) (time & 0xFFFFFFFF);

    }

    cbData.reason = reason;
    cbData.cb_rtn = f;
    cbData.user_data = 0;
    cbData.value = 0;

    this->vpi.vpi_register_cb(&cbData);
    this->checkError();
}

void HdlSimulation::checkReady() {

    while(this->status != SimuStatus::Ready){
        this->checkError(true);
        boost::this_fiber::yield();
    }
    this->checkError(true);
}

void HdlSimulation::checkExecute() {

    while(this->status != SimuStatus::Execute){
        boost::this_fiber::yield();
    }
}

void HdlSimulation::entryPointCbRegister(VpiFnTable table){

    printf("Registering VPI function pointer table \n");

    this->vpi = table;

    printf("Registering entryPoint callback\n");

    this->registerCb([](p_cb_data p_cb) -> PLI_INT32 {
            return simulation->beginCb(p_cb);
            },
            cbStartOfSimulation,
            -1);

    this->checkError();
    printf("Registering end callback \n");

    this->registerCb([](p_cb_data p_cb) -> PLI_INT32 {
            return simulation->endCb(p_cb);
            },
            cbEndOfSimulation,
            -1);

    this->checkError();
    printf("Registering delay callback \n");

    this->registerCb([](p_cb_data p_cb) -> PLI_INT32 {
            return simulation->delayROCb(p_cb);
            },
            cbAfterDelay,
            0);

    this->checkError();
    this->status = SimuStatus::Execute;
}


PLI_INT32 HdlSimulation::beginCb(p_cb_data  data){

    (void) data;
    printf("Start of simulation \n");

    this->checkError();
    return 0;
}

PLI_INT32 HdlSimulation::endCb(p_cb_data  data){

    (void) data;
    printf("End of simulation \n");

    this->status = SimuStatus::Terminated;

    this->checkError();
    return 0;
}

PLI_INT32 HdlSimulation::delayRWCb(p_cb_data  data){

    (void) data;
    this->registerCb([](p_cb_data p_cb) -> PLI_INT32 {
            return simulation->readWriteCb(p_cb);
            },
            cbReadWriteSynch,
            0);

    this->checkError();
    return 0;
}

PLI_INT32 HdlSimulation::delayROCb(p_cb_data  data){

    (void) data;
    this->registerCb([](p_cb_data p_cb) -> PLI_INT32 {
            return simulation->readOnlyCb(p_cb);
            },
            cbReadOnlySynch,
            0);

    this->checkError();
    return 0;
}

PLI_INT32 HdlSimulation::readWriteCb(p_cb_data  data){

    (void) data;
    this->status = SimuStatus::Ready;
    this->checkExecute();

    return 0;
}

PLI_INT32 HdlSimulation::readOnlyCb(p_cb_data  data){

    (void) data;
    this->registerCb([](p_cb_data p_cb) -> PLI_INT32 {
            return simulation->delayRWCb(p_cb);
            },
            cbAfterDelay,
            0);

    this->checkError();
    return 0;
}

void HdlSimulation::printNetInModule(vpiHandle module_handle){
    char* module_name = this->vpi.vpi_get_str(vpiName, module_handle);
    this->vpi.vpi_printf((PLI_BYTE8*)" Signals of %s :\n", module_name);
    vpiHandle net_iterator = this->vpi.vpi_iterate(vpiNet,module_handle);
    if(net_iterator){
        while( vpiHandle netHandle = this->vpi.vpi_scan(net_iterator)){
            std::string NetFullName = this->vpi.vpi_get_str(vpiFullName, netHandle);
            this->vpi.vpi_printf((PLI_BYTE8*)"   %s\n",NetFullName.c_str());
            vpiHandle net = this->vpi.vpi_handle_by_name(const_cast<char*>(NetFullName.c_str()),(vpiHandle) NULL);
            std::string  NetFullName2 = this->vpi.vpi_get_str(vpiFullName,net);
            if (NetFullName.compare(NetFullName2)){
                assert(0);
            }
        }
    } else {
        this->vpi.vpi_printf((PLI_BYTE8*)"   No handles.\n");
    }

    this->vpi.vpi_printf((PLI_BYTE8*)"\n");
}

size_t HdlSimulation::printSignals(){

    vpiHandle topModIterator;
    vpiHandle topModHandle;
    topModIterator = this->vpi.vpi_iterate(vpiModule,NULL);

    if( !topModIterator ){
        return 0;
    }

    topModHandle = this->vpi.vpi_scan(topModIterator);
    while(topModHandle) {
        this->printNetInModule(topModHandle);
        vpiHandle module_iterator = this->vpi.vpi_iterate(vpiModule,topModHandle);
        if (module_iterator){
            vpiHandle module_handle;
            module_handle = this->vpi.vpi_scan(module_iterator);
            while (module_handle) {
                this->printNetInModule(module_handle);
                module_handle = this->vpi.vpi_scan(module_iterator);
            }
        }

        topModHandle = this->vpi.vpi_scan(topModIterator);
    }
    return 0;
}


void HdlSimulation::checkError(bool justThrow){

#ifdef VPI_PLUGIN_NO_ERROR_CHECK
    (void) justThrow;
#else
    if((!justThrow) & (this->status != SimuStatus::Error)) {
        s_vpi_error_info err;
        if (this->vpi.vpi_chk_error(&err)) {
            if(err.level == vpiError) {
                this->errorStr = "VPI error from GHDL : ";
                this->errorStr += err.message;
                this->status = SimuStatus::Error;
                if(this->callerId != boost::this_fiber::get_id()) {
                    boost::this_fiber::yield();
                }
            } else {
                printf("VPI message : %s\n", err.message);
            }
        }
    }

    if((this->callerId == boost::this_fiber::get_id()) &
            (this->status == SimuStatus::Error)){
        throw std::runtime_error(this->errorStr);
    } else if (this->status == SimuStatus::Error) boost::this_fiber::yield();
#endif
}

HdlSimulation::~HdlSimulation(){}
