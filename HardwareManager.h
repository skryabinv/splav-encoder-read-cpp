#pragma once

#include <cstdint>
#include "MessagePackage.h"

#include <atomic>
#include <mutex>

// Управляет gpio
// Читает данные энкодера

struct PinsConfig {
    uint8_t encA{17};
    uint8_t encB{22};
    uint8_t encZ{27};
    uint8_t re485{23};
    uint8_t de485{24};

    uint8_t outPower27V{25};
    uint8_t duplicateEncZ{13};
};

// Здесь все в системном порядке байтов
struct ModbusData {
    float angleOffsetDegrees{};
    float angleAdjDegrees{};
    uint16_t posCountMax{2500};
    uint16_t status{};
    uint16_t power27V{};
};

class HardwareManager {
public:
    HardwareManager(const PinsConfig& config);
    ~HardwareManager();
    // Обновить полученные по modbus конфигурационные значения
    void setModbusData(const ModbusData& data);
    // Обновить данные посылки
    void loadSensorDataPacketTo(SensorDataPacket& data) const;
    // Получить текущий угол энкодера в радианнах
    float getEncoderAngleRad() const;
    uint16_t getAbsEncoderCounter() const;
    uint32_t getEncoderCounter() const;
    ModbusData getModbusData() const;
    void enableReceiver() const;
    void disableReceiver() const;
    void enableTransmiter() const;
    void disableTransmiter() const;
private:
    static void encoderA_ISR(int gpio, int level, uint32_t tick, void *userdata);    
    static void encoderB_ISR(int gpio, int level, uint32_t tick, void *userdata);    
    static void encoderZ_ISR(int gpio, int level, uint32_t tick, void * userdata);
    void processEncoderStep(uint8_t a, uint8_t b);
    // Без блокировки
    float getEncoderAngleRadImpl() const;
    PinsConfig _config;
    ModbusData _modbusData;
    std::atomic_uint32_t _counter{0};
    std::atomic_uint8_t _lastEncoded{0};
    std::atomic_uint16_t _absCounter{0};
    std::atomic_uint8_t _nulPos{};
    // Мютекс для данных modbus
    mutable std::mutex _mutex;
};