#ifndef CONFIG_H
#define CONFIG_H

// -------------------------
// ПАРАМЕТРИ КОНВЕЄРА
// -------------------------
#define BELT_PITCH_MM         2.0     // Крок ременя GT2 (мм)
#define PULLEY_TEETH          20      // Кількість зубів на шківі
#define MICROSTEPS            8       // Дріблення кроку (1/8)
#define MOTOR_STEPS_PER_REV   200     // Кроків на оберт двигуна (звичайно 200)
// Швидкість конвеєра:
#define BELT_SPEED_MM_PER_S   50.0    // Бажана швидкість у мм/с

// -------------------------
// ОБЧИСЛЕННЯ КІНЕМАТИКИ
// -------------------------
// Кроків на мм руху
#define STEPS_PER_MM ((MOTOR_STEPS_PER_REV * MICROSTEPS) / (BELT_PITCH_MM * PULLEY_TEETH))
// Загальна кількість кроків на секунду
#define STEPS_PER_SECOND (STEPS_PER_MM * BELT_SPEED_MM_PER_S)
// Інтервал між кроками в мікросекундах
#define STEP_INTERVAL_MICROS (1000000.0 / STEPS_PER_SECOND)
// Тривалість STEP імпульсу
#define PULSE_WIDTH_MICROS 10
// Напрямки моторів
#define MOTOR_X_DIR LOW // Напрямок мотора X
#define MOTOR_Y_DIR HIGH // Напрямок мотора Y

// -------------------------
// ДАТЧИКИ ТА КНОПКИ
// -------------------------
#define SENSOR_POLL_INTERVAL      10    // мс, інтервал опитування датчиків

#define JAR_CENTERING_MM 8.0 // На скільки мм зрушити баночку вперед після спрацювання датчика

//#define DISPENSE_MODE 0 // 0 - розлив усіх, 1 - розлив лише першої з 6 у збірці
#define JARS_IN_SET   6 // Кількість баночок у збірці

// Рівень відладки:
// 0 - нічого не виводити
// 1 - базова інформація про стан роботи (машина, конвеєр, кроки, режим розливу)
// 2 - повна детальна відладка (усі кнопки, датчики, клапани і т.д.)
#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL 0
#endif

// Enable serial debug output: 1 = enabled, 0 = disabled
#ifndef ENABLE_SERIAL_DEBUG
#define ENABLE_SERIAL_DEBUG 0
#endif

#endif
