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
// ОБЧИСЛЕННЯ КІНЕМАТИКИ
// -------------------------

// Кроків на мм руху (XY): ремінь/шків
#define STEPS_PER_MM_XY ((MOTOR_STEPS_PER_REV_XY * MICROSTEPS_XY) / (BELT_PITCH_MM_XY * PULLEY_TEETH_XY))
// Загальна кількість кроків на секунду (XY)
#define STEPS_PER_SECOND_XY (STEPS_PER_MM_XY * BELT_SPEED_XY_MM_PER_S)
// Інтервал між кроками в мікросекундах (XY)
#define STEP_INTERVAL_XY_MICROS (1000000.0 / STEPS_PER_SECOND_XY)

// Розрахунки Z видалено
// Тривалість STEP імпульсу
#define PULSE_WIDTH_MICROS 10
// Напрямки моторів
#define MOTOR_X_DIR LOW // Напрямок мотора X
// MOTOR_Y_DIR тимчасово не використовується: два мотори на одному драйвері X
// Щоб відновити — розкоментуйте рядок нижче
// #define MOTOR_Y_DIR HIGH // Напрямок мотора Y
// Напрямок мотора Z перенесено

// -------------------------
// ДАТЧИКИ ТА КНОПКИ
// -------------------------
#define SENSOR_POLL_INTERVAL      10    // мс, інтервал опитування датчиків

#define JAR_CENTERING_MM 8.0 // На скільки мм зрушити баночку вперед після спрацювання датчика //8мм

// Логіка другого конвеєра (Z) по датчику 3 перенесена

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
