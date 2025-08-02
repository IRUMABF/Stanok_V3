// For RAMPS 1.4 
// мотор конвеєра x 
#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38

// мотор конвеєра y
#define Y_STEP_PIN         60
#define Y_DIR_PIN          61
#define Y_ENABLE_PIN       56

// мотор екструзії фарби
#define Z_STEP_PIN         46
#define Z_DIR_PIN          48
#define Z_ENABLE_PIN       62

//мотор крана подачі фарби (резервний на майбутнє)
#define E0_STEP_PIN        26
#define E0_DIR_PIN         28
#define E0_ENABLE_PIN      24

//не використовується поки, але залишено для можливого використання
#define E1_STEP_PIN        36
#define E1_DIR_PIN         34
#define E1_ENABLE_PIN      30

//кінцеві вимикачі
#define X_MIN_PIN           3 
#define X_MAX_PIN           2
#define Y_MIN_PIN          14 //датчик наявності баночки під соплом роливу фарби
#define Y_MAX_PIN          15 //датчик наявності баночки під прессом закривання кришки
//#define Z_MIN_PIN          18
//#define Z_MAX_PIN          19
// панель управління
#define BUTTON_PIN         18 // кнопка для запуску/зупинки станка  підключено до Z_MIN_PIN
//#define GND_BUTTON_PIN     17 // пін для підключення кнопки до GND
#define modeButtonPin      19 // кнопка для перемикання режимів роботи підключено до Z_MAX_PIN
//#define ledGndPin          25 // пін для підключення світлодіода індикації режимів роботи до GND
#define ledMode0Pin        20 // світлодіод для індикації режиму 0 (розлив усіх баночок)
#define ledMode1Pin        21 // світлодіод для індикації режиму 1 (розлив лише першої баночки у збірці)

#define servo0Pin          11 // сервопривід для подачі спайок баночок

#define LED_PIN            13 // світлодіод індикації роботи

#define FAN_PIN            9 //резервний, не використовується

#define PS_ON_PIN          12	//ATX , awake=LOW, SLEEP=High

#define HEATER_0_PIN	10  // резервний, не використовується
#define PNEUMATIC_00_PIN	9  // пневматичний клапан для закривання кришки баночки з фарбою
#define PNEUMATIC_01_PIN	8   // пневматичний клапан для завертання кришки

#define TEMP_0_PIN		13   // ANALOG NUMBERING резервний, не використовується
#define TEMP_1_PIN		14   // ANALOG NUMBERING резервний, не використовується

#define PNEUMATIC_1_PIN	31  // резервний, не використовується
#define PNEUMATIC_2_PIN	33  // резервний, не використовується
#define PNEUMATIC_3_PIN	35  // резервний, не використовується
#define PNEUMATIC_4_PIN	37  // резервний, не використовується
#define PNEUMATIC_5_PIN	39  // резервний, не використовується   
#define PNEUMATIC_6_PIN	41  // резервний, не використовується
#define PNEUMATIC_7_PIN	43  // резервний, не використовується
#define PNEUMATIC_8_PIN	45  // резервний, не використовується
#define PNEUMATIC_9_PIN	47  // резервний, не використовується
#define PNEUMATIC_10_PIN	32  // резервний, не використовується
