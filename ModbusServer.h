#pragma once

#include <string>
#include <memory>

class ModbusServer {
public:
    static constexpr auto PORT = 1502;
    static constexpr auto SLAVE_ID = 255;
    static constexpr auto REGISTERS_COUNT = 32;
    static constexpr auto IP = "192.168.20.56";

    ModbusServer(const std::string& ip, int port, int slaveId);
    ~ModbusServer();
    void start();
    void stop();
private:    
    void processRequest(const uint8_t* query, size_t querySize);
    void runImpl();
    struct Impl;
    std::unique_ptr<Impl> _impl;
};