#pragma once

#include <cstdint>

#pragma pack(push, 1) // Выравнивание по байтам

struct SensorDataPacket {
    uint32_t time;                    // 0..3: Время, DWORD, 0.003 с

    float gyro_x;                     // 4..7: ДУС X, FLOAT, °/с
    float gyro_y_rotated;             // 8..11: ДУС Y повернутый, °/с
    float gyro_z_rotated;             // 12..15: ДУС Z повернутый, °/с

    float accel_x;                    // 16..19: Датчик перегрузки X, м/с²
    float accel_y_rotated;            // 20..23: Датчик перегрузки Y повернутый, м/с²
    float accel_z_rotated;            // 24..27: Датчик перегрузки Z повернутый, м/с²

    uint16_t accel_x_70g;             // 28..29: Датчик перегрузки X (70g), WORD, 0.4938 м/с²/е.к.
    uint8_t roll_zero;                // 30: Бит 5 поля — «Крен 0» (предполагаем, что это 5-й бит байта)            

    float alignment_accel_x_or_pos_x; // 31..34: Результат выставки акселерометра X / Координата X, м
    float alignment_accel_y_or_pos_y; // 35..38: Результат выставки акселерометра Y / Координата Y, м
    float alignment_accel_z_or_pos_z; // 39..42: Результат выставки акселерометра Z / Координата Z, м

    float alignment_gyro_x_or_vel_x;  // 43..46: Результат выставки ДУС X / Скорость X, м/с
    float alignment_gyro_y_or_vel_y;  // 47..50: Результат выставки ДУС Y / Скорость Y, м/с
    float alignment_gyro_z_or_vel_z;  // 51..54: Результат выставки ДУС Z / Скорость Z, м/с

    float angle_roll;                 // 55..58: Угол крена изделия, рад
    float angle_pitch;                // 59..62: Угол тангажа изделия, рад
    float angle_yaw;                  // 63..66: Угол курса изделия, рад

    uint8_t voltage_27v;              // 67: Напряжение питания 27В, BYTE, 0.212 В/е.к.
    uint8_t voltage_b1_b2;            // 68: Напряжение питания Б1-Б2, 0.212 В/е.к.
    uint8_t voltage_b3_b5;            // 69: Напряжение питания Б3-Б5, 0.212 В/е.к.
    uint8_t brp_counter_1;            // 70: Счетчик БРП, е.к.
    uint8_t brp_angle_set_1;          // 71: Задание угла 1 БРП, 0.125°/е.к.
    uint8_t brp_angle_set_2;          // 72: Задание угла 2 БРП
    uint8_t brp_angle_set_3;          // 73: Задание угла 3 БРП
    uint8_t brp_angle_set_4;          // 74: Задание угла 4 БРП
    uint8_t brp_angle_current_1;      // 75: Текущий угол 1 БРП
    uint8_t brp_angle_current_2;      // 76: Текущий угол 2 БРП
    uint8_t brp_angle_current_3;      // 77: Текущий угол 3 БРП
    uint8_t brp_angle_current_4;      // 78: Текущий угол 4 БРП
    uint8_t brp_rudder_current_1;     // 79: Ток руля 1 БРП
    uint8_t brp_rudder_current_2;     // 80: Ток руля 2 БРП
    uint8_t brp_rudder_current_3;     // 81: Ток руля 3 БРП
    uint8_t brp_rudder_current_4;     // 82: Ток руля 4 БРП
    uint8_t brp_counter_2;            // 83: Счетчик БРП (дублирующий?)
    uint8_t temperature;              // 84: Температура, е.к.
    uint8_t barometer;                // 85: Барометр, е.к.

    uint16_t bins_status;             // 86..87: Состояние БИНС, WORD (см. Таблица 2)

    float bna_pos_x;                  // 88..91: Координата Х БНА, м
    float bna_pos_y;                  // 92..95: Координата Y БНА, м
    float bna_pos_z;                  // 96..99: Координата Z БНА, м
    float bna_vel_x;                  // 100..103: Скорость Х БНА, м/с
    float bna_vel_y;                  // 104..107: Скорость Y БНА, м/с
    float bna_vel_z;                  // 108..111: Скорость Z БНА, м/с

    int16_t alignment_angle_roll_zero; // 112..113: Угол юстировки «КРЕН 0», м.з.р: 1/212 рад
};

struct MessagePackage {        
    uint8_t start = 0x65;
    // Channel5Data data;
    SensorDataPacket data;
    uint16_t csum;
    uint8_t footer[2] = {0x45, 0xCF};
};

#pragma pack(pop) // Восстановление выравнивания по умолчанию
