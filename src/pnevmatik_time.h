#ifndef PNEVMATIK_TIME_H
#define PNEVMATIK_TIME_H

// -------------------------
// ПНЕВМАТИЧНІ ОПЕРАЦІЇ (часи в мс)
// -------------------------

// === 1. ВИДАЧА СПАЙОК (Valve 1) ===
#define SPICE_OUT_HOLD_TIME      2000  // утримання відкритим для видачі спайок (2 сек)

// === 2. ЦИКЛ РОЗЛИВУ ФАРБИ ===
// Відкриття крану резервуару фарби (Valve 2)
#define PAINT_VALVE_HOLD_TIME    1000  // утримання відкритим для наливу фарби (1 сек)

// Забір фарби поршнем (Valve 3)
#define PAINT_PISTON_IN_HOLD_TIME   800   // утримання поршня в положенні забору

// Видача фарби поршнем (Valve 3)
#define PAINT_PISTON_OUT_HOLD_TIME  1000  // утримання поршня в положенні видачі

// === 3. ЦИКЛ ЗАКРИВАННЯ КРИШОК ===
// Завертання кришок баночок (Valve 4)
#define TWIST_CAP_HOLD_TIME      1800  // утримання в положенні завертання (довше за Valve 5)

// Закривання кришок баночок (Valve 5)
#define CLOSE_CAP_HOLD_TIME      1200  // утримання в положенні закривання (вимкнеться раніше за Valve 4)

// === 4. ЗСУВАННЯ СПАЙКИ ДЛЯ ПАКУВАННЯ ===
// Зсування спайки для пакування (Valve 6)
#define SPICE_SHIFT_HOLD_TIME    500   // утримання в положенні зсування

// === 5. ЦИКЛ ПАКУВАННЯ ===
// Переміщення платформи з присосками (Valve 7)
#define PLATFORM_MOVE_HOLD_TIME  600   // утримання платформи в положенні

// Опускання/піднімання платформи з присосками (Valve 8)
#define PLATFORM_LIFT_HOLD_TIME  500   // утримання платформи в положенні

// Засування спайок в пакет (Valve 9)
#define SPICE_PUSH_HOLD_TIME     600   // утримання циліндра в положенні засування

// Утримання спайок і пакета на платформі (Valve 10)
#define HOLD_SPICE_HOLD_TIME     500   // утримання в положенні

// Рух сопла для вакуумування/запайки (Valve 11)
#define NOZZLE_MOVE_HOLD_TIME    500   // утримання сопла в положенні

// Опускання/піднімання силіконової планки для запайки (Valve 12)
#define SILICONE_BAR_HOLD_TIME   800   // утримання планки в положенні

// Скидання готового пакету (Valve 13)
#define PACKAGE_DROP_HOLD_TIME   400   // утримання в положенні скидання

// Охолодження ленти після запайки (Valve 14)
#define TAPE_COOL_HOLD_TIME      1000  // утримання в положенні охолодження

// === 6. ДОДАТКОВІ ТАЙМЕРИ ДЛЯ АЛГОРИТМУ ===
// Таймери для циклів роботи
#define PAINT_CYCLE_TOTAL_TIME   5000  // загальний час циклу розливу фарби
#define CAP_CYCLE_TOTAL_TIME     4000  // загальний час циклу закривання кришок
#define PACKAGING_CYCLE_TOTAL_TIME 15000 // загальний час циклу пакування

// Таймери для датчиків
#define SENSOR_DEBOUNCE_TIME     50    // час стабілізації датчика
#define SENSOR_TIMEOUT_TIME      10000 // таймаут очікування спрацювання датчика

// Таймери для конвеєра
#define CONVEYOR_START_DELAY     200   // затримка запуску конвеєра
#define CONVEYOR_STOP_DELAY      100   // затримка зупинки конвеєра
#define CONVEYOR_OVERRUN_TIME    1000  // час додаткового руху після зупинки

// Таймери для вакууму
#define VACUUM_BUILD_TIME        1000  // час створення вакууму
#define VACUUM_HOLD_TIME         2000  // час утримання вакууму
#define VACUUM_RELEASE_TIME      500   // час скидання вакууму

// Таймери для нагріву та охолодження
#define HEATER_WARMUP_TIME       3000  // час прогріву нагрівача
#define HEATER_ACTIVE_TIME       2000  // час активного нагріву
#define COOLER_ACTIVE_TIME       3000  // час активного охолодження

// === 7. КОНСТАНТИ ДЛЯ ЛОГІКИ РОБОТИ ===
// Кількість баночок у збірці
#define JARS_IN_SET             6     // кількість баночок у збірці
#define SPICE_SETS_PER_PACKAGE  4     // кількість спайок на пакет

// Режими роботи
#define DISPENSE_MODE_ALL       1     // розлив усіх баночок
#define DISPENSE_MODE_ONE       0     // розлив лише першої баночки

// Стани циклів
#define CYCLE_IDLE              0     // цикл неактивний
#define CYCLE_ACTIVE            1     // цикл активний
#define CYCLE_COMPLETED         2     // цикл завершено

// === 7.1. МІЖКРОКОВІ ПАУЗИ (налаштування пауз між послідовними командами) ===
// Всі значення в мілісекундах. Ці паузи виконуються ПІСЛЯ команди і ПЕРЕД переходом до наступної.
// Налаштовуються тут для зручності, без використання delay(), щоб вся система продовжувала оновлення.

// 0. Видача спайок
#define STEP_PAUSE_SPICE_OUT_MS                 5000

// 1-4. Розлив фарби (послідовність кроків 1..4)
#define STEP_PAUSE_PAINT_VALVE_OPEN_MS          5000
#define STEP_PAUSE_PAINT_PISTON_INTAKE_MS       5000
#define STEP_PAUSE_PAINT_VALVE_CLOSE_MS         5000
#define STEP_PAUSE_PAINT_PISTON_DISPENSE_MS     5000


// 5-6. Закривання кришок
#define STEP_PAUSE_CAP_SCREW_MS                 150   // коротка пауза перед запуском Valve 5 після Valve 4
#define STEP_PAUSE_CAP_CLOSE_MS                 100   // мінімальна пауза після Valve 5

// 7. Зсування спайки
#define STEP_PAUSE_SPICE_SHIFT_MS               5000

// 8-14. Рух платформи / пакетування – підготовчі етапи
#define STEP_PAUSE_PLATFORM_HOME_MS             5000
#define STEP_PAUSE_PLATFORM_DOWN_MS             5000
#define STEP_PAUSE_VACUUM_ON_MS                 5000
#define STEP_PAUSE_PLATFORM_UP_MS               5000
#define STEP_PAUSE_PLATFORM_MOVE_WITH_PACKET_MS 5000
#define STEP_PAUSE_PLATFORM_DOWN_OVER_PACKET_MS 5000
#define STEP_PAUSE_PLATFORM_UP_OPEN_PACKET_MS   5000

// 15-23. Запихання спайок та запайка
#define STEP_PAUSE_PUSH_SPICE_IN_PACKET_MS      5000
#define STEP_PAUSE_HOLDER_EXTEND_MS             5000
#define STEP_PAUSE_NOZZLE_BACK_MS               5000
#define STEP_PAUSE_VACUUM_SEAL_MS               5000
#define STEP_PAUSE_SEALER_DOWN_MS               5000
#define STEP_PAUSE_HEATER_ON_MS                 5000
#define STEP_PAUSE_VACUUM_OFF_MS                5000
#define STEP_PAUSE_SEALER_UP_MS                 5000
#define STEP_PAUSE_COOLER_ON_MS                 5000
#define STEP_PAUSE_HOLDER_UP_MS                 5000
#define STEP_PAUSE_NOZZLE_FORWARD_MS            5000
#define STEP_PAUSE_PUSHER_HOME_MS               5000

// 27-28. Видача готової продукції
#define STEP_PAUSE_PLATFORM_RETURN_HOME_MS      5000
#define STEP_PAUSE_DROP_PACKET_MS               5000

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