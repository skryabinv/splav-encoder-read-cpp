// modbus_server.cpp
#include <iostream>
#include <signal.h>
#include <thread>
#include <pigpio.h>
#include "ModbusServer.h"
#include "MessageSender.h"
#include "HardwareManager.h"

volatile bool running{true};

int main()
{
    const auto PIN = 25;
    if (gpioInitialise() < 0) {
        std::cerr << "Ошибка инициализации pigpio!\n";
        return 1;
    }
    HardwareManager hardware{{}};
    ModbusServer modbus{ModbusServer::IP, ModbusServer::PORT, ModbusServer::SLAVE_ID};
    modbus.start(&hardware);
    MessageSender sender("/dev/serial0");
    sender.start(&hardware);
    signal(SIGINT, [](int) { running = false; });
    while (running) {        
        std::this_thread::sleep_for(std::chrono::milliseconds{1000});        
    }    
    gpioTerminate();
    return 0;
}