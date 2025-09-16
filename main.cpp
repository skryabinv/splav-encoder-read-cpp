// modbus_server.cpp
#include <iostream>
#include <signal.h>
#include <thread>
#include <pigpio.h>
#include "ModbusServer.h"
#include "MessageSender.h"
#include "HardwareManager.h"

constexpr int PORT = 1502;
constexpr int SLAVE_ID = 255;
constexpr int REGISTERS_COUNT = 32;
constexpr auto IP = "192.168.20.56";

volatile bool running{true};

int main()
{
    const auto PIN = 25;
    if (gpioInitialise() < 0) {
        std::cerr << "Ошибка инициализации pigpio!\n";
        return 1;
    }
    HardwareManager hm{{}};
    ModbusServer modbus{IP, PORT, SLAVE_ID};
    modbus.start(&hm);
    MessageSender sender("/dev/serial0");
    sender.start(&hm);
    signal(SIGINT, [](int) { running = false; });
    while (running)
    {        
        std::this_thread::sleep_for(std::chrono::milliseconds{100});
        std::cout << hm.getEncoderAngleRad() << std::endl;                
    }    
    gpioTerminate();
    return 0;
}