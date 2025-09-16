#pragma once

#include <cstdint>

#pragma pack(push, 1) // Выравнивание по байтам

struct Channel5Data
{       
    // Байты 1-3: Время
    uint8_t time[3]; // 0.003 с
    
    // Байты 4-7: ДУС X
    float dus_x; // °/с
    
    // Байты 8-11: ДУС Y
    float dus_y; // °/с
    
    // Байты 12-15: ДУС Z
    float dus_z; // °/с
    
    // Байты 16-19: Датчик перегрузки X
    float acceleration_x; // м/с²
    
    // Байты 20-23: Датчик перегрузки Y
    float acceleration_y; // м/с²
    
    // Байты 24-27: Датчик перегрузки Z
    float acceleration_z; // м/с²
    
    // Байты 28-29: Датчик перегрузки X (70g)
    uint16_t acceleration_x_70g; // 0.4938 м/с²/е.к.
    
    // Байт 30: Ошибки датчика
    uint8_t sensor_errors;
    
    // Байт 31: «Крен 0»
    uint8_t kren_0; // см. таблицу 10.2
    
    // Байты 32-35: Результат выставки акселерометра X / Координата X
    float accel_calib_x_or_coord_x; // м
    
    // Байты 36-39: Результат выставки акселерометра Y / Координата Y
    float accel_calib_y_or_coord_y; // м
    
    // Байты 40-43: Результат выставки акселерометра Z / Координата Z
    float accel_calib_z_or_coord_z; // м
    
    // Байты 44-47: Результат выставки ДУС X / Скорость X
    float dus_calib_x_or_velocity_x; // м/с
    
    // Байты 48-51: Результат выставки ДУС Y / Скорость Y
    float dus_calib_y_or_velocity_y; // м/с
    
    // Байты 52-55: Результат выставки ДУС Z / Скорость Z
    float dus_calib_z_or_velocity_z; // м/с
    
    // Байты 56-59: Угол крена изделия
    float roll_angle; // рад
    
    // Байты 60-63: Угол тангажа изделия
    float pitch_angle; // рад
    
    // Байты 64-67: Угол курса изделия
    float yaw_angle; // рад
    
    // Байт 68: Напряжение питания 27В
    uint8_t voltage_27v; // 0.212 В/е.к.
    
    // Байт 69: Напряжение питания Б1-Б2
    uint8_t voltage_b1_b2; // 0.212 В/е.к.
    
    // Байт 70: Напряжение питания Б3-Б5
    uint8_t voltage_b3_b5; // 0.212 В/е.к.
    
    // Байт 71: Счетчик БРП
    uint8_t brp_counter;
    
    // Байт 72: Задание угла 1 БРП
    uint8_t brp_angle_command_1; // 0.125°/е.к.
    
    // Байт 73: Задание угла 2 БРП
    uint8_t brp_angle_command_2; // 0.125°/е.к.
    
    // Байт 74: Текущий угол 1 БРП
    uint8_t brp_current_angle_1; // 0.125°/е.к.
    
    // Байт 75: Текущий угол 2 БРП
    uint8_t brp_current_angle_2; // 0.125°/е.к.
    
    // Байт 76: Ток руля 1 БРП
    uint8_t brp_current_1;
    
    // Байт 77: Ток руля 2 БРП
    uint8_t brp_current_2;
    
    // Байт 78: Счетчик БРП (дублирование?)
    uint8_t brp_counter_2;
    
    // Байт 79: Температура
    uint8_t temperature;
    
    // Байт 80: Барометр
    uint8_t barometer;
    
    // Байты 81-82: Слово состояния БИНС
    uint16_t bins_state_word; // Таблица 8.3
    
    // Байты 83-84: Задание привода БИНС
    uint16_t bins_drive_command;
    
    // Байты 85-86: ДУС изделия
    uint16_t product_dus;
    
    // Байты 87-88: Угол двигателя БИНС
    uint16_t bins_motor_angle;
    
    // Байты 89-92: Результат выставки акселерометра Z 70
    float accel_calib_z_70; // М
    
    // Байты 93-96: Координата X БНА
    float bna_coord_x; // М
    
    // Байты 97-100: Координата Y БНА
    float bna_coord_y; // М
    
    // Байты 101-104: Координата Z БНА
    float bna_coord_z; // М
    
    // Байты 105-108: Скорость X БНА
    float bna_velocity_x; // м/с
    
    // Байты 109-112: Скорость Y БНА
    float bna_velocity_y; // м/с
    
    // Байты 113-116: Скорость Z БНА
    float bna_velocity_z; // м/с
    
    // Контрольная сумма и окончание (0x45, 0xCF) - не входят в DATA
};

struct MessagePackage {        
    uint8_t start = 0x65;
    Channel5Data data;
    uint8_t csum;
    uint8_t footer[2] = {0x45, 0xCF};
};

#pragma pack(pop) // Восстановление выравнивания по умолчанию