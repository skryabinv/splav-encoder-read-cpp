#pragma once

#include <memory>
#include <string>
#include <chrono>
#include <stop_token>

struct Channel5Data;

class MessageSender {
public:    
    // TODO: Объект для обновления данных
    // Функция обновления данных    
    MessageSender(const std::string& serial,
        std::chrono::milliseconds interval = std::chrono::milliseconds{3});
    ~MessageSender();
    // Вызывается из главного потока
    void start();
    // Вызывается из главного потока
    void stop();    
private:    
    void runImpl(std::stop_token stoken);
    struct Impl;
    std::unique_ptr<Impl> _impl;
};