#pragma once

#include <memory>
#include <string>
#include <chrono>
#include <stop_token>

struct Channel5Data;
class HardwareManager;

class MessageSender {
public:    
    // TODO: Объект для обновления данных
    // Функция обновления данных    
    MessageSender(const std::string& serial,
        std::chrono::microseconds interval = std::chrono::microseconds{2800});
    ~MessageSender();
    // Вызывается из главного потока
    void start(HardwareManager* manager);
    // Вызывается из главного потока
    void stop();    
    // Функция вычисления контрольной суммы
    static uint16_t computeCRC(const uint8_t* data, size_t length);
private:    
    void runImpl(std::stop_token stoken, HardwareManager* manager);
    uint16_t controlSum() const;
    struct Impl;
    std::unique_ptr<Impl> _impl;
};