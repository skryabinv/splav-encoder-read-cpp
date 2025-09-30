#include "HardwareManager.h"
#include <pigpio.h>
#include <arpa/inet.h>

HardwareManager::HardwareManager(const PinsConfig& config) : _config{config} {
    gpioSetMode(config.encA, PI_INPUT);
    gpioSetMode(config.encB, PI_INPUT);
    gpioSetMode(config.encZ, PI_INPUT);

    gpioSetPullUpDown(config.encA, PI_PUD_UP);
    gpioSetPullUpDown(config.encB, PI_PUD_UP);
    gpioSetPullUpDown(config.encZ, PI_PUD_DOWN);

    _lastEncoded.store((gpioRead(config.encA) << 1) | gpioRead(config.encB), std::memory_order_relaxed);
    gpioSetAlertFuncEx(config.encA, HardwareManager::encoderA_ISR, this);
    gpioSetAlertFuncEx(config.encB, HardwareManager::encoderB_ISR, this);
    gpioSetAlertFuncEx(config.encZ, HardwareManager::encoderZ_ISR, this);

    gpioSetMode(config.outPower27V, PI_OUTPUT);
    // Вначале отключаем
    gpioWrite(config.outPower27V, 1);

    gpioSetMode(config.re485, PI_OUTPUT);    
    gpioSetMode(config.de485, PI_OUTPUT);    

    gpioSetMode(config.duplicateEncZ, PI_INPUT);
    gpioSetPullUpDown(config.duplicateEncZ, PI_PUD_DOWN);
    gpioSetAlertFuncEx(config.duplicateEncZ, HardwareManager::encoderZ_ISR, this);
    _nulPos = std::max(gpioRead(config.encZ), gpioRead(config.duplicateEncZ));
}

HardwareManager::~HardwareManager() = default;

void HardwareManager::setModbusData(const ModbusData &data) {
    auto power27V = [this, &data]{
        auto lock = std::scoped_lock{_mutex};
        _modbusData = data;
        return data.power27V;
    }();    
    gpioWrite(_config.outPower27V, power27V == 0 ? 1 : 0);
}

void HardwareManager::loadSensorDataPacketTo(SensorDataPacket &data) const {
    constexpr float scale = (std::numbers::pi_v<float> / 180.0f) * (1 << 12);    
    auto lock = std::scoped_lock{_mutex};
    // "КРЕН 0" - Бит 5 (Состояние входа)
    data.roll_zero = _nulPos.load(std::memory_order_relaxed) << 5;
    // Угол крена изделия
    data.angle_roll = getEncoderAngleRadImpl();
    // Состояние БИНС
    data.bins_status = _modbusData.status;    
    // Угол юстировки "КРЕН 0" (1/2^12 рад)
    data.alignment_angle_roll_zero = static_cast<int16_t>(_modbusData.angleAdjDegrees * scale);    
}

float HardwareManager::getEncoderAngleRad() const {        
    auto lock = std::scoped_lock{_mutex};
    return getEncoderAngleRadImpl();
}

uint16_t HardwareManager::getAbsEncoderCounter() const {
    return _absCounter.load(std::memory_order_relaxed);
}

uint32_t HardwareManager::getEncoderCounter() const {
    return _counter.load(std::memory_order_relaxed);
}

ModbusData HardwareManager::getModbusData() const {
    auto lock = std::scoped_lock{_mutex};
    return _modbusData;
}

void HardwareManager::encoderA_ISR(int gpio, int level, uint32_t tick, void *userdata) {
    auto manager = static_cast<HardwareManager*>(userdata);    
    int b = (manager->_lastEncoded.load() & 1); 
    static_cast<HardwareManager*>(userdata)->processEncoderStep(level, b);
}

void HardwareManager::encoderB_ISR(int gpio, int level, uint32_t tick, void *userdata) {
    auto manager = static_cast<HardwareManager*>(userdata);    
    int a = (manager->_lastEncoded.load() >> 1) & 1;
    static_cast<HardwareManager*>(userdata)->processEncoderStep(a, level);
}


void HardwareManager::encoderZ_ISR(int gpio, int level, uint32_t tick, void * userdata) {
    if (level == 1) {  
        // Change to High
        static_cast<HardwareManager*>(userdata)->_counter.store(0);  
        static_cast<HardwareManager*>(userdata)->_nulPos = 1;
    } else {
        static_cast<HardwareManager*>(userdata)->_nulPos = 0;
    }
}

void HardwareManager::enableReceiver() const {
    gpioWrite(_config.re485, 0);
}

void HardwareManager::disableReceiver() const {
    gpioWrite(_config.re485, 1);
}

void HardwareManager::enableTransmiter() const {
    gpioWrite(_config.de485, 1);
}

void HardwareManager::disableTransmiter() const {
    gpioWrite(_config.de485, 0);
}

void HardwareManager::processEncoderStep(uint8_t a, uint8_t b) {    
    // Кодируем состояние: A << 1 | B
    int current = (a << 1) | b;

    // Предыдущее закодированное состояние
    uint8_t prev = _lastEncoded.load();

    // Собираем 4-битное значение: предыдущее + текущее
    int sum = (prev << 2) | current;

    // Таблица переходов: определяет направление
    // https://en.wikipedia.org/wiki/Quadrature_encoding   
    static const int8_t transitions[16] = {
        [0b0000] = 0,
        [0b0001] = -1,
        [0b0010] = +1,
        [0b0011] = 0, // error
        
        [0b0100] = +1,
        [0b0101] = 0,
        [0b0110] = 0, // error
        [0b0111] = -1,
    
        [0b1000] = -1,
        [0b1001] = 0, // error
        [0b1010] = 0,
        [0b1011] = +1,
    
        [0b1100] = 0, // error
        [0b1101] = +1,
        [0b1110] = -1,
        [0b1111] = 0    
    };
    int8_t delta = transitions[sum];  
    if (delta != 0) {         
        _counter.fetch_add(delta, std::memory_order_relaxed);
    } 
    // Сохраняем текущее состояние
    _lastEncoded.store(current, std::memory_order_relaxed);
    _absCounter.fetch_add(1, std::memory_order_relaxed);
}

float HardwareManager::getEncoderAngleRadImpl() const {
    auto signedCounter = static_cast<int32_t>(getEncoderCounter());    
    auto offsetRad = std::numbers::pi * _modbusData.angleOffsetDegrees / 180.0f;    
    return offsetRad + 2.0f * std::numbers::pi * signedCounter / _modbusData.posCountMax;  
}
