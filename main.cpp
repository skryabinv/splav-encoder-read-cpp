// modbus_server.cpp
#include <iostream>
#include <signal.h>
#include <thread>
#include <pigpio.h>
#include "ModbusServer.h"
#include "MessageSender.h"

constexpr int PORT = 1502;
constexpr int SLAVE_ID = 255;
constexpr int REGISTERS_COUNT = 32;
constexpr auto IP = "192.168.20.56";

volatile bool running{true};

int main() {
    const auto PIN = 25;
    // if (gpioInitialise() < 0) {
    //     std::cerr << "Ошибка инициализации pigpio!\n";
    //     return 1;
    // }
    // gpioSetMode(PIN, PI_OUTPUT);

    ModbusServer modbus{IP, PORT, SLAVE_ID};
    modbus.start();
    MessageSender sender("/dev/serial0");
    sender.start();
    signal(SIGINT, [](int) { running = false; });

    while(running) {
        // gpioWrite(PIN, 1);    
        std::this_thread::sleep_for(std::chrono::milliseconds{50});
        // gpioWrite(PIN, 0);
        std::this_thread::sleep_for(std::chrono::milliseconds{50});
    }
    modbus.stop();
    // gpioTerminate();    
    return 0;
}