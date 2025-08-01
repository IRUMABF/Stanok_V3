#ifndef PNEUMATIK_TIME_H
#define PNEUMATIK_TIME_H
// -------------------------
// ПНЕВМАТИЧНІ ОПЕРАЦІЇ (часи в мс)
// -------------------------

// 1. Видача спайок (№1)
#define SPICE_OUT_OPEN_TIME      120   // відкривання циліндра для видачі спайок
#define SPICE_OUT_HOLD_TIME      300   // утримання відкритим для видачі спайок
#define SPICE_OUT_DELAY          100   // затримка після видачі спайок

// 2. Відкриття крану резервуару фарби (№2)
#define PAINT_VALVE_OPEN_TIME    100   // відкривання крану резервуару фарби
#define PAINT_VALVE_HOLD_TIME    500   // утримання відкритим для наливу фарби
#define PAINT_VALVE_DELAY        100   // затримка після відкриття

// 3. Забір фарби поршнем (№3)
#define PAINT_PISTON_IN_OPEN_TIME   150   // рух поршня для забору фарби
#define PAINT_PISTON_IN_HOLD_TIME   400   // утримання поршня в положенні забору
#define PAINT_PISTON_IN_DELAY       100   // затримка після забору

// 4. Закриття крану резервуару фарби (№2)
#define PAINT_VALVE_CLOSE_TIME   100   // закриття крану резервуару фарби
#define PAINT_VALVE_CLOSE_HOLD   300   // утримання закритим
#define PAINT_VALVE_CLOSE_DELAY  100   // затримка після закриття

// 5. Видача фарби поршнем (№3)
#define PAINT_PISTON_OUT_OPEN_TIME  150   // рух поршня для видачі фарби
#define PAINT_PISTON_OUT_HOLD_TIME  400   // утримання поршня в положенні видачі
#define PAINT_PISTON_OUT_DELAY      100   // затримка після видачі

// 6. Завертання кришок баночок (№4)
#define TWIST_CAP_OPEN_TIME      150   // відкривання циліндра для завертання кришок
#define TWIST_CAP_HOLD_TIME      200   // утримання в положенні завертання
#define TWIST_CAP_DELAY          100   // затримка після завертання

// 7. Закривання кришок баночок (№5)
#define CLOSE_CAP_OPEN_TIME      600   // відкривання циліндра для закривання кришок
#define CLOSE_CAP_HOLD_TIME      300   // утримання в положенні закривання
#define CLOSE_CAP_DELAY          100   // затримка після закривання

// 8. Зсування спайки для пакування (№6)
#define SPICE_SHIFT_OPEN_TIME    200   // відкривання циліндра для зсування спайки
#define SPICE_SHIFT_HOLD_TIME    300   // утримання в положенні зсування
#define SPICE_SHIFT_DELAY        100   // затримка після зсування

// 9. Переміщення платформи з присосками (№7)
#define PLATFORM_MOVE_OPEN_TIME  250   // переміщення платформи з присосками
#define PLATFORM_MOVE_HOLD_TIME  400   // утримання платформи в положенні
#define PLATFORM_MOVE_DELAY      100   // затримка після переміщення

// 10. Опускання/піднімання платформи з присосками (№8)
#define PLATFORM_LIFT_OPEN_TIME  200   // опускання/піднімання платформи з присосками
#define PLATFORM_LIFT_HOLD_TIME  300   // утримання платформи в положенні
#define PLATFORM_LIFT_DELAY      100   // затримка після операції

// 11. Засування спайок в пакет (№9)
#define SPICE_PUSH_OPEN_TIME     250   // засування спайок в пакет
#define SPICE_PUSH_HOLD_TIME     400   // утримання циліндра в положенні засування
#define SPICE_PUSH_DELAY         100   // затримка після засування

// 12. Утримання спайок і пакета на платформі (№10)
#define HOLD_SPICE_OPEN_TIME     150   // висування утримання спайок і пакета
#define HOLD_SPICE_HOLD_TIME     300   // утримання в положенні
#define HOLD_SPICE_DELAY         100   // затримка після утримання

// 13. Рух сопла для вакуумування/запайки (№11)
#define NOZZLE_MOVE_OPEN_TIME    200   // рух сопла до пакету
#define NOZZLE_MOVE_HOLD_TIME    300   // утримання сопла в положенні
#define NOZZLE_MOVE_DELAY        100   // затримка після руху

// 14. Опускання/піднімання силіконової планки для запайки (№12)
#define SILICONE_BAR_OPEN_TIME   200   // опускання/піднімання силіконової планки
#define SILICONE_BAR_HOLD_TIME   400   // утримання планки в положенні
#define SILICONE_BAR_DELAY       100   // затримка після операції

// 15. Скидання готового пакету (№13)
#define PACKAGE_DROP_OPEN_TIME   150   // відкривання циліндра для скидання пакету
#define PACKAGE_DROP_HOLD_TIME   300   // утримання в положенні скидання
#define PACKAGE_DROP_DELAY       100   // затримка після скидання

// 16. Охолодження ленти після запайки (№14)
#define TAPE_COOL_OPEN_TIME      300   // відкривання циліндра для охолодження ленти
#define TAPE_COOL_HOLD_TIME      500   // утримання в положенні охолодження
#define TAPE_COOL_DELAY          100   // затримка після охолодження

#endif