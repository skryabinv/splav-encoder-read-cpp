#include "MessageSender.h"
#include "MessagePackage.h"
#include "HardwareManager.h"

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <thread>
#include <iostream>
#include <stdexcept>
#include <chrono>

// Сконфигурировать вызывающий поток как реал-тайм
static bool configure_realtime(pthread_t thread_id) {
    sched_param sch = {1};
    if (pthread_setschedparam(thread_id, SCHED_FIFO, &sch)) {
        return false;
    }
    cpu_set_t cpu_set;
    CPU_ZERO(&cpu_set);
    CPU_SET(3, &cpu_set);
    if (pthread_setaffinity_np(thread_id, sizeof(cpu_set), &cpu_set)) {
        return false;
    }
    return true;
}


struct MessageSender::Impl {
    // Порт
    std::string serial;   
    // Данные
    MessagePackage data{};    
    // Дескриптор
    int sfd{-1};
    // Интервал отправки
    std::chrono::microseconds interval;
    // Функция для обновления данных
    // Поток
    std::jthread _thread;    
    ~Impl() {
        if(sfd != -1) {
            close(sfd);    
        }
    }
    std::uint32_t time{};
};

MessageSender::MessageSender(const std::string &serial, std::chrono::microseconds interval) {    
    _impl = std::make_unique<Impl>();
    _impl->serial = serial;
    _impl->interval = interval;    
    std::cout << "Открытие порта: " << serial << "..." << std::endl;
    _impl->sfd = open(serial.data(), O_WRONLY | O_NOCTTY);
    if(_impl->sfd == -1) {
        std::cerr << "Не удалось открыть порт: " << serial << std::endl;
        throw std::runtime_error("Ошибка открытия последовательного порта");
    }    
    termios tty;
    tcgetattr(_impl->sfd, &tty);
    tty.c_iflag = 0;
    tty.c_oflag = 0;
    tty.c_lflag = 0;
    cfsetospeed(&tty, B500000);
    cfsetispeed(&tty, B500000);

    tty.c_cflag &= ~PARENB; // Без контроля четности
    tty.c_cflag &= ~CSTOPB; // 1 стоп-бит
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8; // 8 бит данных
    tcsetattr(_impl->sfd, TCSANOW, &tty);    
    std::cout << "Порт " << serial << " открыт" << std::endl;
}

MessageSender::~MessageSender() {
    stop();
}

void MessageSender::start(HardwareManager* manager) {
    if(manager == nullptr) {
        std::cerr << "Менеджер оборудования не создан" << std::endl;
        return;
    }
    stop();    
    std::cout << "Запуск потока последовательного порта " << _impl->serial << std::endl;
    _impl->_thread = std::jthread(&MessageSender::runImpl, this, manager);
}

void MessageSender::stop() {
    if(_impl->_thread.joinable()) {
        _impl->_thread.request_stop();
        _impl->_thread.join();
    }
}

uint16_t MessageSender::computeCRC(const uint8_t *data, size_t length) {
    static constexpr uint16_t POLY = 0xA097;
    static constexpr uint16_t INIT = 0x0000;
    uint16_t crc = INIT;
    for (size_t i = 0; i < length; ++i) {
        crc ^= static_cast<uint16_t>(data[i]) << 8;  // Сдвигаем байт в старшую часть
        for (int j = 0; j < 8; ++j) {
            if (crc & 0x8000) {
                crc = static_cast<uint16_t>((crc << 1) ^ POLY);
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

void MessageSender::runImpl(std::stop_token stoken, HardwareManager* manager) {    
    configure_realtime(pthread_self());    
    manager->enableTransmiter();         
    while(!stoken.stop_requested()) {          
        auto t1 = std::chrono::steady_clock::now();                      
        manager->loadSensorDataPacketTo(_impl->data.data);  
        _impl->data.csum = controlSum();
        _impl->data.data.time = ++_impl->time;
        auto written = write(_impl->sfd, &_impl->data, sizeof(_impl->data));        
        if(written != sizeof(_impl->data)) {
            std::cerr << "Ошибка записи в последовательный порт: " << written << std::endl;
        }
        auto t2 = std::chrono::steady_clock::now();        
        auto remains = t2 - t1;
        if(remains < _impl->interval) {
            std::this_thread::sleep_for(_impl->interval - remains);
        } else {
            std::cerr << "Превышено максимальное время отправки" << std::endl;
        }        
    }        
}

uint16_t MessageSender::controlSum() const {
    constexpr auto size = sizeof(SensorDataPacket);
    auto ptr = reinterpret_cast<const uint8_t*>(&_impl->data.data);
    return computeCRC(ptr, size);    
}
