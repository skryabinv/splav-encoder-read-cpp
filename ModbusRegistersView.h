#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <algorithm>

class ModbusRegistersView {
public:   

    // Конструктор из сырого указателя и размера
    ModbusRegistersView(int baseAddress, uint16_t* data, int size)
        : _baseAddress{baseAddress}, _data(data), _size(size > 0 ? size : 0) {}    

    // Чтение uint16_t по адресу (сохраняет порядок байт как в протоколе)
    uint16_t readUint16(size_t address) const {
        auto offset = address - _baseAddress;
        check_bounds(offset, 1);
        return _data[offset];
    }

    // Изменяет порядок байт на системный
    uint16_t readUint16Litle(size_t address) const {        
        return __builtin_bswap16(readUint16(address));
    }

    // Запись uint16_t по адресу
    void writeUint16(size_t address, uint16_t value) {
        auto offset = address - _baseAddress;
        check_bounds(offset, 1);
        _data[offset] = value;
    }

    // Чтение int16_t по адресу
    int16_t readInt16(size_t address) const {
        return static_cast<int16_t>(readUint16(address));
    }

    // Изменяет порядок байт на системный
    int16_t readInt16Litle(size_t address) const {        
        return static_cast<int16_t>(readUint16Litle(address));
    }


    // Запись int16_t по адресу
    void writeInt16(size_t address, int16_t value) {
        writeUint16(address, static_cast<uint16_t>(value));
    }

    // Чтение float (из двух регистров, big-endian)
    float readFloat(size_t address) const {
        auto offset = address - _baseAddress;
        check_bounds(offset, 2);
        uint32_t combined = (static_cast<uint32_t>(_data[offset]) << 16) | _data[offset + 1];
        float result;
        std::memcpy(&result, &combined, sizeof(result));
        return result;
    }

    // Запись float (в два регистра, big-endian)
    void writeFloat(size_t address, float value) {        
        auto offset = address - _baseAddress;
        check_bounds(offset, 2);
        uint32_t bits;
        std::memcpy(&bits, &value, sizeof(bits));
        _data[offset]     = static_cast<uint16_t>((bits >> 16) & 0xFFFF);
        _data[offset + 1] = static_cast<uint16_t>(bits & 0xFFFF);
    }   

    // Получить размер
    size_t size() const { return _size; }

private:
    uint16_t* _data{};
    size_t _size{};
    int _baseAddress{};

    // Проверка границ
    void check_bounds(size_t address, size_t count) const {
        if (address + count > _size) {
            throw std::out_of_range("Modbus register access out of bounds");
        }
    }
};