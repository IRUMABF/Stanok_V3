#ifndef PNEVMATIK_TIME_H
#define PNEVMATIK_TIME_H

// -------------------------
// ПНЕВМАТИЧНІ ОПЕРАЦІЇ (часи в мс)
// -------------------------
// 0. Видача спайок
#define SPICE_OUT_HOLD_TIME                 1000

// 1-4. Розлив фарби (послідовність кроків 1..4)
#define STEP_PAUSE_PAINT_VALVE_OPEN_MS          1000
#define STEP_PAUSE_PAINT_PISTON_INTAKE_MS       1000
#define STEP_PAUSE_PAINT_VALVE_CLOSE_MS         1000
#define STEP_PAUSE_PAINT_PISTON_DISPENSE_MS     1000


// Закривання кришок
#define STEP_PAUSE_CAP_SCREW_MS                 300   // пауза перед запуском Valve 5 після Valve 4
#define CLOSE_CAP_HOLD_TIME                     800  // утримання в положенні закривання (вимкнеться раніше за Valve 4)
#define STEP_PAUSE_CAP_CLOSE_MS                 300   // мінімальна пауза після Valve 5


// Зсування спайки для пакування (Valve 6)
#define SPICE_SHIFT_HOLD_TIME    1000   // утримання в положенні зсування

// === 6. ДОДАТКОВІ ТАЙМЕРИ ДЛЯ АЛГОРИТМУ ===
// Таймери для циклів роботи
#define PAINT_CYCLE_TOTAL_TIME   5000  // загальний час циклу розливу фарби
#define CAP_CYCLE_TOTAL_TIME     4000  // загальний час циклу закривання кришок

// Таймери для датчиків
#define SENSOR_DEBOUNCE_TIME     50    // час стабілізації датчика
#define SENSOR_TIMEOUT_TIME      10000 // таймаут очікування спрацювання датчика

// === 7. КОНСТАНТИ ДЛЯ ЛОГІКИ РОБОТИ ===
// Кількість баночок у збірці
#define JARS_IN_SET             6     // кількість баночок у збірці
#define SPICE_SETS_PER_PACKAGE  4     // кількість спайок на пакет




// === 8. МАКРОСИ ДЛЯ ЗРУЧНОСТІ ===
// Макрос для перетворення секунд в мілісекунди
#define SECONDS_TO_MS(seconds) ((seconds) * 1000)

// Макрос для перетворення мілісекунд в секунди
#define MS_TO_SECONDS(ms) ((ms) / 1000.0)

// Макрос для перевірки таймауту
#define IS_TIMEOUT(start_time, timeout) (millis() - (start_time) > (timeout))

// Макрос для перевірки інтервалу
#define IS_INTERVAL(start_time, interval) (millis() - (start_time) > (interval))

#endif