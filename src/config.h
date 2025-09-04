#ifndef CONFIG_H
#define CONFIG_H

// -------------------------
// ПАРАМЕТРИ КОНВЕЄРА (XY) — основний конвеєр з ременем/шківом
// -------------------------
#define BELT_PITCH_MM_XY         2.0     // Крок ременя GT2 (мм)
#define PULLEY_TEETH_XY          20      // Кількість зубів на шківі
#define MICROSTEPS_XY            8       // Дріблення кроку (1/8)
#define MOTOR_STEPS_PER_REV_XY   200     // Кроків на оберт двигуна (звичайно 200)
// Швидкість конвеєра XY:
#define BELT_SPEED_XY_MM_PER_S   50.0    // Бажана швидкість у мм/с

// -------------------------
// ПАРАМЕТРИ КОНВЕЄРА (Z) — другий конвеєр з гладким валиком
// -------------------------
#define ROLLER_DIAMETER_Z_MM     40.0    // Діаметр валика Z (мм)
#define MICROSTEPS_Z             8       // Дріблення кроку (1/8)
#define MOTOR_STEPS_PER_REV_Z    200     // Кроків на оберт двигуна (звичайно 200)
// Швидкість конвеєра Z:
#define BELT_SPEED_Z_MM_PER_S    150.0    // Бажана швидкість у мм/с для Z

// -------------------------
// ОБЧИСЛЕННЯ КІНЕМАТИКИ
// -------------------------
// Константа PI (Arduino може не мати M_PI)
#ifndef CFG_PI
#define CFG_PI 3.14
#endif

// Кроків на мм руху (XY): ремінь/шків
#define STEPS_PER_MM_XY ((MOTOR_STEPS_PER_REV_XY * MICROSTEPS_XY) / (BELT_PITCH_MM_XY * PULLEY_TEETH_XY))
// Загальна кількість кроків на секунду (XY)
#define STEPS_PER_SECOND_XY (STEPS_PER_MM_XY * BELT_SPEED_XY_MM_PER_S)
// Інтервал між кроками в мікросекундах (XY)
#define STEP_INTERVAL_XY_MICROS (1000000.0 / STEPS_PER_SECOND_XY)

// Довжина кола валика Z
#define ROLLER_CIRCUMFERENCE_Z_MM (CFG_PI * ROLLER_DIAMETER_Z_MM)
// Кроків на мм руху (Z): валик
#define STEPS_PER_MM_Z ((MOTOR_STEPS_PER_REV_Z * MICROSTEPS_Z) / (ROLLER_CIRCUMFERENCE_Z_MM))
// Загальна кількість кроків на секунду (Z)
#define STEPS_PER_SECOND_Z (STEPS_PER_MM_Z * BELT_SPEED_Z_MM_PER_S)
// Інтервал між кроками в мікросекундах (Z)
#define STEP_INTERVAL_Z_MICROS (1000000.0 / STEPS_PER_SECOND_Z)
// Тривалість STEP імпульсу
#define PULSE_WIDTH_MICROS 10
// Напрямки моторів
#define MOTOR_X_DIR LOW // Напрямок мотора X
#define MOTOR_Y_DIR HIGH // Напрямок мотора Y
// Напрямок мотора Z (другий конвеєр)
#define MOTOR_Z_DIR LOW

// -------------------------
// ДАТЧИКИ ТА КНОПКИ
// -------------------------
#define SENSOR_POLL_INTERVAL      10    // мс, інтервал опитування датчиків

#define JAR_CENTERING_MM 8.0 // На скільки мм зрушити баночку вперед після спрацювання датчика

// -------------------------
// ДРУГИЙ КОНВЕЄР (Z) ПО ДАТЧИКУ 3
// -------------------------
// Увімкнення логіки чергування зсувів по датчику 3 для другого конвеєра
#ifndef CONVEYOR_Z_SENSOR3_SHIFT_ENABLED
#define CONVEYOR_Z_SENSOR3_SHIFT_ENABLED 1
#endif

// Зміщення (мм) для першої баночки після спрацювання датчика 3
#ifndef CONVEYOR_Z_OFFSET_MM_FIRST
#define CONVEYOR_Z_OFFSET_MM_FIRST 10.0
#endif

// Зміщення (мм) для другої баночки після спрацювання датчика 3
#ifndef CONVEYOR_Z_OFFSET_MM_SECOND
#define CONVEYOR_Z_OFFSET_MM_SECOND 15.0
#endif

// Мінімальний інтервал між обробками фронтів датчика 3 (мс) для захисту від дребезгу
#ifndef CONVEYOR_Z_MIN_TRIGGER_INTERVAL_MS
#define CONVEYOR_Z_MIN_TRIGGER_INTERVAL_MS 50
#endif

//#define DISPENSE_MODE 0 // 0 - розлив усіх, 1 - розлив лише першої з 6 у збірці
// JARS_IN_SET визначено у pnevmatik_time.h як частина логіки алгоритму

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
