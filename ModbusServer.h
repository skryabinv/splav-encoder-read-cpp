#pragma once

#include <string>
#include <memory>

class HardwareManager;

class ModbusServer {
public:
    static constexpr auto PORT = 1502;
    static constexpr auto SLAVE_ID = 255;
    static constexpr auto REGISTERS_COUNT = 32;
    static constexpr auto IP = "192.168.199.56";    

    ModbusServer(const std::string& ip, int port, int slaveId);
    ~ModbusServer();
    void start(HardwareManager* manager);
    void stop();
private:    
    void processRequest(const uint8_t* query, size_t querySize, HardwareManager* manager);
    void runImpl(HardwareManager* manager);
    struct Impl;
    std::unique_ptr<Impl> _impl;
};