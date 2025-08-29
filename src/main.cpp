#include <Arduino.h>
#include "pinout.h"
#include "config.h"
#include "pnevmatik_time.h"
#include "vacuumValve.h"
#include "pneumatic_valve.h"
#include "controls.h"
#include "conveyor.h"

// Створення об'єктів для кожного клапана
PneumaticValve valve1(PNEUMATIC_1_PIN);
PneumaticValve valve2(PNEUMATIC_2_PIN);
PneumaticValve valve3(PNEUMATIC_3_PIN);
PneumaticValve valve4(PNEUMATIC_4_PIN);
PneumaticValve valve5(PNEUMATIC_5_PIN); 
PneumaticValve valve6(PNEUMATIC_6_PIN); 
PneumaticValve valve7(PNEUMATIC_7_PIN);
PneumaticValve valve8(PNEUMATIC_8_PIN);
PneumaticValve valve9(PNEUMATIC_9_PIN);
PneumaticValve valve10(PNEUMATIC_10_PIN);
PneumaticValve valve11(PNEUMATIC_11_PIN);
PneumaticValve valve12(PNEUMATIC_12_PIN);
PneumaticValve valve13(PNEUMATIC_13_PIN);
PneumaticValve valve14(PNEUMATIC_14_PIN);
Controls controls;
Conveyor conveyor;

// --- Прототипи команд ---
// 1. Видача спайок
void commandSpiceOut();

// 2. Видача фарби (розбито на підетапи для наладки)
void commandPaintValveOpen();
void commandPaintPistonIntake();
void commandPaintValveClose();
void commandPaintPistonDispense();

// 3. Закриття кришок (розбито на підетапи)
void commandCapScrew();
void commandCapClose();

// 4. Зсування спайки в положення для пакування
void commandSpiceShift();

// 5. Пересування платформи з присосками для захвату пакету (розбито на підетапи)
void commandPlatformHome();
void commandPlatformDown();
void commandVacuumOn();
void commandPlatformUp();
void commandPlatformMoveWithPacket();
void commandPlatformDownOverPacket();
void commandPlatformUpOpenPacket();

// 6. Запихання спайок в пакет і запайка (розбито на підетапи)
void commandPushSpiceInPacket();
void commandHolderExtend();
void commandNozzleBack();
void commandVacuumSeal();
void commandSealerDown();
void commandHeaterOn();
void commandVacuumOff();
void commandSealerUp();
void commandCoolerOn();
void commandHolderUp();
void commandNozzleForward();
void commandPusherHome();

// 7. Видача готової продукції (розбито на підетапи)
void commandPlatformReturnHome();
void commandDropPacket();

// --- Масив команд (приклад, порядок і склад змінюйте під свій цикл) ---
typedef void (*MachineCommand)();

MachineCommand machineProgram[] = {
    commandSpiceOut,
    commandPaintValveOpen,
    commandPaintPistonIntake,
    commandPaintValveClose,
    commandPaintPistonDispense,
    commandCapScrew,
    commandCapClose,
    commandSpiceShift,
    commandPlatformHome,
    commandPlatformDown,
    commandVacuumOn,
    commandPlatformUp,
    commandPlatformMoveWithPacket,
    commandPlatformDownOverPacket,
    commandPlatformUpOpenPacket,
    commandPushSpiceInPacket,
    commandHolderExtend,
    commandNozzleBack,
    commandVacuumSeal,
    commandSealerDown,
    commandHeaterOn,
    commandVacuumOff,
    commandSealerUp,
    commandCoolerOn,
    commandHolderUp,
    commandNozzleForward,
    commandPusherHome,
    commandPlatformReturnHome,
    commandDropPacket
};
const uint8_t machineProgramLength = sizeof(machineProgram) / sizeof(machineProgram[0]);

bool machineRunning = false;
bool singleBlockMode = false;
uint8_t currentStep = 0;
bool dispenseMode = false; // режим розливу (true - всі, false - лише перша)
bool machinePaused = false; // true — все стоїть, false — все працює

void setup() {
  Serial.begin(9600);
  vacuumValveInit();
  setVacuumValve(VALVE_POS_1);
  valve1.begin();
  valve2.begin();
  valve3.begin();
  valve4.begin();
  valve5.begin();
  valve6.begin();
  valve7.begin();
  valve8.begin();
  valve9.begin();
  valve10.begin();
  valve11.begin();
  valve12.begin();
  valve13.begin();
  valve14.begin();
  controls.begin();
  conveyor.begin();
  // початкове положення клапанів 
  //valve1.on();
  //valve2.off();
  //valve3.on();
  Serial.println("Setup complete");
}

void loop() {
    controls.update();

    // --- Кнопка старт ---
    if (controls.startPressed()) {
        machinePaused = false;
        conveyor.enable();
        machineRunning = true;
    }

    // --- Кнопка стоп ---
    if (controls.stopPressed()) {
        machinePaused = true;
        conveyor.disable();
        machineRunning = false;
    }

    // --- Кнопка single block ---
    if (controls.singleBlockPressed()) {
        singleBlockMode = !singleBlockMode;
        currentStep = 0;
        machineRunning = false;
    }

    // --- Кнопка зміни режиму розливу ---
    if (controls.modeChanged()) {
        dispenseMode = !dispenseMode;
        if (dispenseMode) {
            Serial.println("Dispense mode: All");
            
        } else {
            Serial.println("Dispense mode: Single");

        }
        // ...індикація...
    }

    // --- Оновлення механізмів тільки якщо не пауза ---
    if (!machinePaused) {
        conveyor.update();
        // ...оновлення клапанів, таймерів...
    }

    // --- Логіка виконання команд (single block/auto) ---
    if (!machinePaused) {
        if (singleBlockMode) {
            // ...single block логіка...
        } else if (machineRunning) {
            // ...автоматичний режим...
        }
    }
}

// --- Допоміжні функції ---

void executeStep(uint8_t step) {
    // Тут реалізуйте виконання однієї команди з алгоритму для single block режиму
    // Наприклад:
    switch (step) {
        case 0:
            // Видача спайок
            break;
        case 1:
            // Видача фарби
            break;
        // ...
        default:
            // Кінець циклу
            break;
    }
}

void toggleDispenseMode() {
    // Тут реалізуйте перемикання режиму розливу (наприклад, перемикання змінної, індикація)
}

void commandSpiceOut() {
    // Видача спайок
    // ...логіка...
}
void commandPaintValveOpen() {
    // Відкриття клапана фарби
    // ...логіка...
}
void commandPaintPistonIntake() {
    // Всмоктування фарби поршнем
    // ...логіка...
}
void commandPaintValveClose() {
    // Закриття клапана фарби
    // ...логіка...
}
void commandPaintPistonDispense() {
    // Видача фарби поршнем
    // ...логіка...
}
void commandCapScrew() {
    // Закручування кришок
    // ...логіка...
}
void commandCapClose() {
    // Закриття кришок
    // ...логіка...
}
void commandSpiceShift() {
    // Зсування спайки
    // ...логіка...
}
void commandPlatformHome() {
    // Повернення платформи в початкове положення
    // ...логіка...
}
void commandPlatformDown() {
    // Опускання платформи
    // ...логіка...
}
void commandVacuumOn() {
    // Увімкнення вакууму
    // ...логіка...
}
void commandPlatformUp() {
    // Піднімання платформи
    // ...логіка...
}
void commandPlatformMoveWithPacket() {
    // Пересування платформи з пакетом
    // ...логіка...
}
void commandPlatformDownOverPacket() {
    // Опускання платформи над пакетом
    // ...логіка...
}
void commandPlatformUpOpenPacket() {
    // Піднімання платформи з відкриттям пакета
    // ...логіка...
}
void commandPushSpiceInPacket() {
    // Запихання спайок в пакет
    // ...логіка...
}
void commandHolderExtend() {
    // Витягування тримача
    // ...логіка...
}
void commandNozzleBack() {
    // Рух сопла назад
    // ...логіка...
}
void commandVacuumSeal() {
    // Вакуумна запайка
    // ...логіка...
}
void commandSealerDown() {
    // Опускання запайника
    // ...логіка...
}
void commandHeaterOn() {
    // Увімкнення нагрівача
    // ...логіка...
}
void commandVacuumOff() {
    // Вимкнення вакууму
    // ...логіка...
}
void commandSealerUp() {
    // Піднімання запайника
    // ...логіка...
}
void commandCoolerOn() {
    // Увімкнення охолоджувача
    // ...логіка...
}
void commandHolderUp() {
    // Піднімання тримача
    // ...логіка...
}
void commandNozzleForward() {
    // Рух сопла вперед
    // ...логіка...
}
void commandPusherHome() {
    // Повернення штовхача в початкове положення
    // ...логіка...
}
void commandPlatformReturnHome() {
    // Повернення платформи додому
    // ...логіка...
}
void commandDropPacket() {
    // Скидання пакета
    // ...логіка...
}