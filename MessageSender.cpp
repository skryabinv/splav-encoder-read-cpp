#include "MessageSender.h"
#include "Channel5Data.h"

#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

#include <thread>
#include <iostream>
#include <stdexcept>
#include <chrono>


struct MessageSender::Impl {
    // Порт
    std::string serial;
    // Данные
    MessagePackage data{};    
    // Дескриптор
    int sfd{-1};
    // Интервал отправки
    std::chrono::milliseconds interval;
    // Функция для обновления данных
    // Поток
    std::jthread _thread;    
    ~Impl() {
        if(sfd != -1) {
            close(sfd);    
        }
    }
};

MessageSender::MessageSender(const std::string &serial, std::chrono::milliseconds interval) {    
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

void MessageSender::start() {
    stop();    
    std::cout << "Запуск потока последовательного порта " << _impl->serial << std::endl;
    _impl->_thread = std::jthread(&MessageSender::runImpl, this);
}

void MessageSender::stop() {
    if(_impl->_thread.joinable()) {
        _impl->_thread.request_stop();
        _impl->_thread.join();
    }
}

void MessageSender::runImpl(std::stop_token stoken) {    
    while(!stoken.stop_requested()) {            
        auto t1 = std::chrono::steady_clock::now();  
        // Обновить данные
        // Высчитать crc      
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