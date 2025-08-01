#ifndef PINOUT_H
#define PINOUT_H
// For RAMPS 1.4 
#define PNEUMATIC_1_PIN	17  // видача спайок (розподілювач №1)
#define PNEUMATIC_2_PIN	23  // відкриття/закриття крану резервуару фарби (розподілювач №2)
#define PNEUMATIC_3_PIN	16  // рух поршня для забору/видачі фарби (розподілювач №3)
#define PNEUMATIC_4_PIN	9   // завертання кришок баночок (розподілювач №4)
#define PNEUMATIC_5_PIN	8   // закривання кришок баночок (розподілювач №5)
#define PNEUMATIC_6_PIN	25  // зсування спайки для пакування (розподілювач №6)
#define PNEUMATIC_7_PIN	27  // переміщення платформи з присосками (розподілювач №7)
#define PNEUMATIC_8_PIN	29  // опускання/піднімання платформи з присосками (розподілювач №8)
#define PNEUMATIC_9_PIN	31  // засування спайок в пакет (розподілювач №9)
#define PNEUMATIC_10_PIN	33 // утримання спайок і пакета на платформі (розподілювач №10)
#define PNEUMATIC_11_PIN	35 // рух сопла для вакуумування/запайки (розподілювач №11)
#define PNEUMATIC_12_PIN	37 // опускання/піднімання силіконової планки для запайки (розподілювач №12)
#define PNEUMATIC_13_PIN	39 // скидання готового пакету (розподілювач №13)
#define PNEUMATIC_14_PIN	41 // охолодження ленти після запайки (розподілювач №14)

// мотор конвеєра x 
#define X_STEP_PIN         54
#define X_DIR_PIN          55
#define X_ENABLE_PIN       38

// мотор конвеєра y
#define Y_STEP_PIN         60
#define Y_DIR_PIN          61
#define Y_ENABLE_PIN       56

//кінцеві вимикачі
#define sensor_1          14 //датчик наявності баночки під соплом роливу фарби(на платі як Y_MIN_PIN)
#define sensor_2          15 //датчик наявності баночки під прессом закривання кришки(на платі як Y_MAX_PIN)
#define sensor_3          2  //датчик чи спайка баночок готова до здвигання (на платі як X_MAX_PIN)
// панель управління
#define start_PIN         18 // кнопка для запуску станка  підключено до Z_MIN_PIN
#define stop_PIN         3 // кнопка для зупинки станка  підключено до X_MIN_PIN
#define modeButtonPin      19 // кнопка для перемикання режимів роботи підключено до Z_MAX_PIN
#define singleblockButtonPin  66 // кнопка для виконання по одній операції алгоритму, підключено до A13,на шилді T0
#define ledMode0Pin        20 // світлодіод для індикації режиму 0 (розлив усіх баночок)
#define ledMode1Pin        21 // світлодіод для індикації режиму 1 (розлив лише першої баночки у збірці)
#define LED_PIN            13 // світлодіод індикації роботи(той що на платі як LED_BUILTIN)

// #define FAN_PIN            9 //резервний, не використовується
//#define PS_ON_PIN          12	//ATX , awake=LOW, SLEEP=High
//#define HEATER_0_PIN	10  // резервний, не використовується

#define vacuumValvePin1  57 // клапан вакуумування, сигнал 1 підключено до A3(d57)
#define vacuumValvePin2  58 // клапан вакуумування, сигнал 2 підключено до A4(d58)

#define heaterPin       11 // нагрівач сопла, сигнал підключено до d11, блок servos на платі
//мосфети які вільні підключені до:
#define MOSFET_1_PIN      43
#define MOSFET_2_PIN      45
#define MOSFET_3_PIN      47
#define MOSFET_4_PIN      32

#endif