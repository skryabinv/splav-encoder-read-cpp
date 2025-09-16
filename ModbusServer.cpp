#include "ModbusServer.h"
#include <stdexcept>
#include <atomic>
#include <iostream>
#include <thread>
#include <cstring>

#include "ModbusRegistersView.h"
#include "HardwareManager.h"

extern "C" {
    #include <modbus/modbus.h>
}

enum InputRegisters {
    FBK_Angle_Roll = 1000,
    FBK_Angle_Adj = 1002,
    FBK_Angle = 1004,
    FBK_Pos_Count = 1006,
    FBK_Pos_Count_Max,    
    FBK_Power_27_V,
    Reserve_9,
    Reserve_10,
    Reserve_11,
    Reserve_12,
    Reserve_13,
    Reserve_14,
    Reserve_15,
    Reserve_16,
    Reserve_17,
    Reserve_18,
    Reserve_19,
};

enum HoldingRegisters {
    SP_Angle_Offset = 2000,
    SP_Angle_Adj = 2002,
    SP_Pos_Count_Max = 2004,
    SP_Status,
    SP_Power_27_V,
    SP_reserve_7,
    SP_reserve_8,
    SP_reserve_9,
    SP_reserve_10,
    SP_reserve_11,
    SP_reserve_12,
    SP_reserve_13,
    SP_reserve_14,
    SP_reserve_15,
    SP_reserve_16,
    SP_reserve_17,
    SP_reserve_18,
    SP_reserve_19,
};

struct ModbusServer::Impl {
    std::string ip;
    int port;
    int slaveId;
    modbus_t* modbusCtx{};
    modbus_mapping_t* modbusMap{};
    std::atomic_bool stop{true};    
    std::jthread _thread;
    ~Impl() {        
        modbus_mapping_free(modbusMap);
        modbus_close(modbusCtx);
        modbus_free(modbusCtx);
    }
};

ModbusServer::ModbusServer(const std::string &ip, int port, int slaveId) {
    _impl = std::make_unique<Impl>();
    _impl->ip = ip;
    _impl->port = port;
    _impl->slaveId = slaveId;
    _impl->modbusCtx = modbus_new_tcp(_impl->ip.data(), _impl->port);
    if(!_impl->modbusCtx) {        
        throw new std::runtime_error("Can't create modbus context");
    }    
    _impl->modbusMap = modbus_mapping_new_start_address(0, 0, 0, 0, 2000, 20, 1000, 20);
    if(!_impl->modbusMap) {        
        throw new std::runtime_error("Can't create modbus context");
    }        
}

ModbusServer::~ModbusServer() {
    stop();
}

void ModbusServer::start(HardwareManager* manager) {    
    if(manager == nullptr) {
        std::cerr << "Не инициализирован объект менеджера оборудования" << std::endl;
    }
    stop();
    _impl->_thread = std::jthread{
         [this, manager]{ runImpl(manager); }
    };
}

void ModbusServer::stop() {
    _impl->stop = true;
}

void ModbusServer::processRequest(const uint8_t * query, size_t querySize, HardwareManager* manager) {
    auto functionCode = query[7];        
    uint16_t startAddress = (query[8] << 8) | query[9];               
    uint16_t count = (query[10] << 8) | query[11];              
    auto toRadians = [](float degrees) {
        return std::numbers::pi * degrees / 180.0;
    };
    auto toDegrees = [](float radians) {
        return radians / std::numbers::pi * 180.0;
    };
    auto inputRegs = ModbusRegistersView{1000, _impl->modbusMap->tab_input_registers, _impl->modbusMap->nb_input_registers};
    auto holdingRegs = ModbusRegistersView{2000, _impl->modbusMap->tab_registers, _impl->modbusMap->nb_registers};         
    if(functionCode == MODBUS_FC_WRITE_MULTIPLE_REGISTERS && startAddress == 2000) {
        // Обновление входных регистров по данным из holdingRegisters
        inputRegs.writeFloat(FBK_Angle_Adj, toRadians(holdingRegs.readFloat(SP_Angle_Adj)));        
        inputRegs.writeUint16(FBK_Pos_Count_Max, holdingRegs.readUint16(SP_Pos_Count_Max));        
        inputRegs.writeUint16(FBK_Power_27_V, holdingRegs.readUint16(SP_Power_27_V));                
        // Обновить структуру данных в manager
        ModbusData data;
        data.angleAdj = holdingRegs.readFloat(SP_Angle_Adj);
        data.angleOffset = holdingRegs.readFloat(SP_Angle_Offset);
        // Нулевое значение не должно быть
        if(auto posCountMax = holdingRegs.readUint16Litle(SP_Pos_Count_Max); posCountMax != 0) {
            data.posCountMax = posCountMax;
        }               
        data.power27V = holdingRegs.readUint16Litle(SP_Power_27_V);
        data.status = holdingRegs.readUint16Litle(SP_Status);
        manager->setModbusData(data);
    } 
    if(functionCode == MODBUS_FC_READ_INPUT_REGISTERS || functionCode == MODBUS_FC_READ_HOLDING_REGISTERS) {
        // Обновить счетчик угла
        auto floatRad = manager->getEncoderAngleRad();
        inputRegs.writeFloat(FBK_Angle_Roll, floatRad);       
        inputRegs.writeFloat(FBK_Angle, toDegrees(floatRad));
    }
}

void ModbusServer::runImpl(HardwareManager* manager) {
    _impl->stop = false;    
    constexpr auto connections = 1;
    auto serverSocket = modbus_tcp_listen(_impl->modbusCtx, connections);
    if (serverSocket == -1) {
        std::cerr << "Failed to listen: " << modbus_strerror(errno) << "\n";
        return;
    }
    while(!_impl->stop) {
        std::cout << "Waiting for connection...\n";
        auto clientSocket = modbus_tcp_accept(_impl->modbusCtx, &serverSocket);
        if (clientSocket == -1) {            
            break;
        }
        std::cout << "Client connected.\n";
        while (!_impl->stop) {
            uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
            int request_length = modbus_receive(_impl->modbusCtx, query);
            if (request_length == -1) {
                std::cerr << "Receive failed, closing connection.\n";                            
                break;
            }               
            // Обрабатываем запрос (чтение/запись регистров)
            if (modbus_reply(_impl->modbusCtx, query, request_length, _impl->modbusMap) == -1) {                
                std::cerr << "Reply failed: " << modbus_strerror(errno) << "\n";
                break;
            }
            processRequest(query, request_length, manager);    
            // Сформировать структуру ModbusData
            ModbusData modbusData;            
        }
        close(clientSocket);        
    }        
    close(serverSocket);
    
}
