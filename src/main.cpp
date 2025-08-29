#include <Arduino.h>
#include "pinout.h"
#include "config.h"
#include "pnevmatik_time.h"
#include "vacuumValve.h"
#include "pneumatic_valve.h"
#include "controls.h"
#include "conveyor.h"
#include "sensor_utils.h"

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

// Змінні для відладки
unsigned long lastDebugTime = 0;
const unsigned long DEBUG_INTERVAL = 4000; // 2 секунди між оновленнями відладки

void setup() {
  Serial.begin(9600);
  Serial.println("=== STANOK V3 INITIALIZATION ===");
  
  // Ініціалізація датчиків
  SensorUtils::initSensors();
  
  // Ініціалізація вакуумного клапана
  vacuumValveInit();
  setVacuumValve(VALVE_POS_1);
  
  // Ініціалізація пневматичних клапанів
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
  
  // Ініціалізація управління та конвеєра
  controls.begin();
  conveyor.begin();
  
  // Тестування датчиків
  Serial.println("Testing sensors...");
  SensorUtils::testAllSensors();
  
  // Початкове положення клапанів 
  valve1.on();
  delay(500);
  valve1.off();
  //valve3.on();
  
  Serial.println("=== SETUP COMPLETE ===");
  Serial.println("System ready for operation!");
  Serial.println("Press buttons to test functionality");
  Serial.println("=====================================");
}

void loop() {
    controls.update();
    
    // Оновлення таймерів клапанів (ОБОВ'ЯЗКОВО!)
    valve1.update();
    valve2.update();
    valve3.update();
    valve4.update();
    valve5.update();
    valve6.update();
    valve7.update();
    valve8.update();
    valve9.update();
    valve10.update();
    valve11.update();
    valve12.update();
    valve13.update();
    valve14.update();

    // Отримуємо стан датчиків
    bool sensor1 = SensorUtils::readSensor1();
    bool sensor2 = SensorUtils::readSensor2();
    bool sensor3 = SensorUtils::readSensor3();

    // Отримуємо стан кнопок
    bool startBtn = controls.startPressed();
    bool stopBtn = controls.stopPressed();
    bool modeBtn = controls.modeChanged();
    bool singleBtn = controls.singleBlockPressed();

    // --- Кнопка старт ---
    if (startBtn) {
        if (singleBlockMode) {
            // В single block режимі - виконуємо наступну команду
            if (currentStep < machineProgramLength) {
                Serial.println("=== START BUTTON PRESSED (SINGLE BLOCK) ===");
                Serial.print("Executing step: ");
                Serial.println(currentStep);
                
                // Виконуємо поточну команду
                machineProgram[currentStep]();
                
                // Переходимо до наступної
                currentStep++;
                
                if (currentStep >= machineProgramLength) {
                    Serial.println("=== ALL STEPS COMPLETED ===");
                    singleBlockMode = false;
                    currentStep = 0;
                } else {
                    Serial.print("Next step: ");
                    Serial.println(currentStep);
                }
            }
        } else {
            // Звичайний режим - запускаємо машину
            machinePaused = false;
            conveyor.enable();
            machineRunning = true;
            Serial.println("=== START BUTTON PRESSED ===");
            Serial.println("Machine started - Running mode");
            Serial.println("Conveyor enabled and started");
        }
    }

    // --- Кнопка стоп ---
    if (stopBtn) {
        machinePaused = true;
        conveyor.disable();
        machineRunning = false;
        Serial.println("=== STOP BUTTON PRESSED ===");
        Serial.println("Machine stopped - Paused mode");
        Serial.println("Conveyor disabled and stopped");
    }

    // --- Кнопка single block ---
    if (singleBtn) {
        singleBlockMode = !singleBlockMode;
        currentStep = 0;
        machineRunning = false;
        Serial.println("=== SINGLE BLOCK BUTTON PRESSED ===");
        Serial.print("Single block mode: ");
        Serial.println(singleBlockMode ? "ENABLED" : "DISABLED");
        Serial.println("Current step reset to 0");
        
        if (singleBlockMode) {
            Serial.println("Press START to execute next step");
            Serial.println("Press STOP to pause execution");
        }
    }

    // --- Кнопка зміни режиму розливу ---
    if (modeBtn) {
        dispenseMode = !dispenseMode;
        Serial.println("=== MODE BUTTON PRESSED ===");
        if (dispenseMode) {
            Serial.println("Dispense mode: ALL JARS");
            Serial.println("Conveyor will stop for every jar");
        } else {
            Serial.println("Dispense mode: SINGLE JAR");
            Serial.println("Conveyor will stop only for first jar");
        }
    }

    // --- Тестування клапанів (для демонстрації onFor/offFor) ---
    static unsigned long lastValveTest = 0;
    static bool valveTestState = false;
    
    if (millis() - lastValveTest > 5000) { // Кожні 5 секунд
        lastValveTest = millis();
        valveTestState = !valveTestState;
        
        if (valveTestState) {
            Serial.println("=== VALVE TEST: onFor(1000ms) ===");
            valve1.onFor(1000); // Увімкнути на 1 секунду
            Serial.println("Valve 1 will turn OFF automatically in 1 second");
        } else {
            Serial.println("=== VALVE TEST: offFor(1000ms) ===");
            valve1.offFor(1000); // Вимкнути на 1 секунду
            Serial.println("Valve 1 will turn ON automatically in 1 second");
        }
    }

    // --- Відладка датчиків (кожні 2 секунди) ---
    unsigned long now = millis();
    if (now - lastDebugTime > DEBUG_INTERVAL) {
        lastDebugTime = now;
        
        // Виводимо детальну інформацію в Serial
        Serial.println("=== STATUS UPDATE ===");
        Serial.print("Machine Status: ");
        if (machinePaused) {
            Serial.println("PAUSED");
        } else if (machineRunning) {
            Serial.println("RUNNING");
        } else {
            Serial.println("STOPPED");
        }
        
        Serial.print("Operating Mode: ");
        Serial.println(singleBlockMode ? "SINGLE BLOCK" : "AUTO");
        
        Serial.print("Dispense Mode: ");
        Serial.println(dispenseMode ? "ALL JARS" : "SINGLE JAR");
        
                 Serial.print("Current Step: ");
         Serial.print(currentStep);
         Serial.print("/");
         Serial.println(machineProgramLength);
         
         if (currentStep < machineProgramLength) {
             Serial.print("Next Command: ");
             Serial.println(currentStep == 0 ? "SPICE OUT" : 
                          currentStep == 1 ? "PAINT VALVE OPEN" :
                          currentStep == 2 ? "PAINT PISTON INTAKE" :
                          currentStep == 3 ? "PAINT VALVE CLOSE" :
                          currentStep == 4 ? "PAINT PISTON DISPENSE" :
                          currentStep == 5 ? "CAP SCREW" :
                          currentStep == 6 ? "CAP CLOSE" :
                          currentStep == 7 ? "SPICE SHIFT" :
                          "OTHER");
         }
        
        Serial.print("Conveyor Status: ");
        Serial.println(conveyor.isRunning() ? "RUNNING" : "STOPPED");
        
        Serial.print("Conveyor Dociag: ");
        Serial.println(conveyor.isDociagActive() ? "ACTIVE" : "INACTIVE");
        
        Serial.println("--- SENSOR STATUS ---");
        Serial.print("Sensor 1 (Paint dispense): ");
        Serial.println(sensor1 ? "ACTIVE - Jar detected" : "INACTIVE - No jar");
        
        Serial.print("Sensor 2 (Cap press): ");
        Serial.println(sensor2 ? "ACTIVE - Jar under press" : "INACTIVE - No jar");
        
        Serial.print("Sensor 3 (Spice shift): ");
        Serial.println(sensor3 ? "ACTIVE - Spice ready" : "INACTIVE - Not ready");
        
        Serial.println("--- BUTTON STATUS ---");
        Serial.print("START button: ");
        Serial.println(startBtn ? "PRESSED" : "RELEASED");
        
        Serial.print("STOP button: ");
        Serial.println(stopBtn ? "PRESSED" : "RELEASED");
        
        Serial.print("MODE button: ");
        Serial.println(modeBtn ? "PRESSED" : "RELEASED");
        
                 Serial.print("SINGLE BLOCK button: ");
         Serial.println(singleBtn ? "PRESSED" : "RELEASED");
         
         Serial.println("--- VALVE STATUS ---");
         Serial.print("Valve 1: ");
         Serial.println(valve1.isOn() ? "ON" : "OFF");
         Serial.print("Valve 2: ");
         Serial.println(valve2.isOn() ? "ON" : "OFF");
         Serial.print("Valve 3: ");
         Serial.println(valve3.isOn() ? "ON" : "OFF");
         
         Serial.println("===================");
    }

    // --- Оновлення механізмів тільки якщо не пауза ---
    if (!machinePaused) {
        conveyor.update();
        
        // Додаткова відладка для розуміння стану
        static unsigned long lastStatusCheck = 0;
        if (millis() - lastStatusCheck > 1000) { // Кожну секунду
            lastStatusCheck = millis();
            
            if (singleBlockMode) {
                Serial.print("SINGLE BLOCK: Waiting for START button. Step: ");
                Serial.println(currentStep);
            } else if (machineRunning) {
                Serial.print("AUTO MODE: Running. Step: ");
                Serial.println(currentStep);
            } else {
                Serial.println("Machine idle - press START or enable SINGLE BLOCK");
            }
        }
    }

    // --- Логіка виконання команд (single block/auto) ---
    if (!machinePaused) {
        if (singleBlockMode) {
            // Single block режим - виконуємо по одній команді
            if (currentStep < machineProgramLength) {
                Serial.print("=== SINGLE BLOCK: Executing step ");
                Serial.print(currentStep);
                Serial.println(" ===");
                
                // Виконуємо поточну команду
                machineProgram[currentStep]();
                
                // Переходимо до наступної команди
                currentStep++;
                
                Serial.print("Step completed. Next step: ");
                Serial.println(currentStep);
                
                // Якщо досягли кінця - вимикаємо single block режим
                if (currentStep >= machineProgramLength) {
                    Serial.println("=== SINGLE BLOCK: All steps completed ===");
                    singleBlockMode = false;
                    currentStep = 0;
                }
            }
        } else if (machineRunning) {
            // Автоматичний режим - виконуємо всі команди послідовно
            if (currentStep < machineProgramLength) {
                Serial.print("=== AUTO MODE: Executing step ");
                Serial.print(currentStep);
                Serial.println(" ===");
                
                // Виконуємо поточну команду
                machineProgram[currentStep]();
                
                // Переходимо до наступної команди
                currentStep++;
                
                // Невелика затримка між командами
                delay(100);
                
                // Якщо досягли кінця - перезапускаємо цикл
                if (currentStep >= machineProgramLength) {
                    Serial.println("=== AUTO MODE: Cycle completed, restarting ===");
                    currentStep = 0;
                }
            }
        }
    }
}

// --- Допоміжні функції ---

void executeStep(uint8_t step) {
    // Функція для ручного виконання команд (можна використовувати для тестування)
    if (step < machineProgramLength) {
        Serial.print("=== MANUAL EXECUTION: Step ");
        Serial.print(step);
        Serial.println(" ===");
        
        machineProgram[step]();
        
        Serial.println("Manual execution completed");
    } else {
        Serial.println("Invalid step number!");
    }
}

void toggleDispenseMode() {
    // Тут реалізуйте перемикання режиму розливу (наприклад, перемикання змінної, індикація)
}

void commandSpiceOut() {
    // Видача спайок
    Serial.println("=== EXECUTING: SPICE OUT ===");
    
    // Перевіряємо датчик 3 (спайка готова до здвигання)
    bool sensor3 = SensorUtils::readSensor3();
    Serial.print("Sensor 3 (Spice shift) status: ");
    Serial.println(sensor3 ? "ACTIVE - Spice ready for shifting" : "INACTIVE - Spice not ready");
    
    // Приклад використання onFor - увімкнути клапан на 2 секунди
    Serial.println("Activating spice dispense valve for 2 seconds...");
    valve1.onFor(2000); // Увімкнути на 2 секунди
    
    Serial.println("Spice out operation completed");
    Serial.println("===============================");
}
void commandPaintValveOpen() {
    // Відкриття клапана фарби
    Serial.println("=== EXECUTING: PAINT VALVE OPEN ===");
    
    // Перевіряємо датчик 1 (баночка під соплом розливу фарби)
    bool sensor1 = SensorUtils::readSensor1();
    Serial.print("Sensor 1 (Paint dispense) status: ");
    Serial.println(sensor1 ? "ACTIVE - Jar under paint nozzle" : "INACTIVE - No jar under nozzle");
    
    // Приклад використання offFor - вимкнути клапан на 1 секунду, потім увімкнути
    Serial.println("Closing paint valve for 1 second, then opening...");
    valve2.offFor(1000); // Вимкнути на 1 секунду, потім автоматично увімкнути
    
    Serial.println("Paint valve opened successfully");
    Serial.println("===============================");
}
void commandPaintPistonIntake() {
    // Всмоктування фарби поршнем
    Serial.println("=== EXECUTING: PAINT PISTON INTAKE ===");
    Serial.println("Piston moving to intake position...");
    
    // ...логіка всмоктування фарби...
    
    Serial.println("Paint piston intake completed");
    Serial.println("===============================");
}
void commandPaintValveClose() {
    // Закриття клапана фарби
    Serial.println("=== EXECUTING: PAINT VALVE CLOSE ===");
    Serial.println("Closing paint valve...");
    
    // ...логіка закриття клапана...
    
    Serial.println("Paint valve closed successfully");
    Serial.println("===============================");
}
void commandPaintPistonDispense() {
    // Видача фарби поршнем
    Serial.println("=== EXECUTING: PAINT PISTON DISPENSE ===");
    Serial.println("Piston dispensing paint into jar...");
    
    // ...логіка видачі фарби...
    
    Serial.println("Paint piston dispense completed");
    Serial.println("===============================");
}
void commandCapScrew() {
    // Закручування кришок
    Serial.println("=== EXECUTING: CAP SCREW ===");
    
    // Перевіряємо датчик 2 (баночка під пресом закривання кришки)
    bool sensor2 = SensorUtils::readSensor2();
    Serial.print("Sensor 2 (Cap press) status: ");
    Serial.println(sensor2 ? "ACTIVE - Jar under cap press" : "INACTIVE - No jar under press");
    
    // ...логіка закручування кришок...
    
    Serial.println("Cap screw operation completed");
    Serial.println("===============================");
}
void commandCapClose() {
    // Закриття кришок
    Serial.println("=== EXECUTING: CAP CLOSE ===");
    Serial.println("Closing jar caps...");
    
    // ...логіка закриття кришок...
    
    Serial.println("Cap close operation completed");
    Serial.println("===============================");
}
void commandSpiceShift() {
    // Зсування спайки
    Serial.println("=== EXECUTING: SPICE SHIFT ===");
    Serial.println("Shifting spice set to packaging position...");
    
    // ...логіка зсування спайки...
    
    Serial.println("Spice shift operation completed");
    Serial.println("===============================");
}
void commandPlatformHome() {
    Serial.println("=== EXECUTING: PLATFORM HOME ===");
    Serial.println("Moving platform to home position...");
    // ...логіка...
    Serial.println("Platform home operation completed");
    Serial.println("===============================");
}
void commandPlatformDown() {
    Serial.println("=== EXECUTING: PLATFORM DOWN ===");
    Serial.println("Lowering platform...");
    // ...логіка...
    Serial.println("Platform down operation completed");
    Serial.println("===============================");
}
void commandVacuumOn() {
    Serial.println("=== EXECUTING: VACUUM ON ===");
    Serial.println("Activating vacuum system...");
    // ...логіка...
    Serial.println("Vacuum on operation completed");
    Serial.println("===============================");
}
void commandPlatformUp() {
    Serial.println("=== EXECUTING: PLATFORM UP ===");
    Serial.println("Raising platform...");
    // ...логіка...
    Serial.println("Platform up operation completed");
    Serial.println("===============================");
}
void commandPlatformMoveWithPacket() {
    Serial.println("=== EXECUTING: PLATFORM MOVE WITH PACKET ===");
    Serial.println("Moving platform with packet...");
    // ...логіка...
    Serial.println("Platform move with packet completed");
    Serial.println("===============================");
}
void commandPlatformDownOverPacket() {
    Serial.println("=== EXECUTING: PLATFORM DOWN OVER PACKET ===");
    Serial.println("Lowering platform over packet...");
    // ...логіка...
    Serial.println("Platform down over packet completed");
    Serial.println("===============================");
}
void commandPlatformUpOpenPacket() {
    Serial.println("=== EXECUTING: PLATFORM UP OPEN PACKET ===");
    Serial.println("Raising platform and opening packet...");
    // ...логіка...
    Serial.println("Platform up open packet completed");
    Serial.println("===============================");
}
void commandPushSpiceInPacket() {
    Serial.println("=== EXECUTING: PUSH SPICE IN PACKET ===");
    Serial.println("Pushing spice set into packet...");
    // ...логіка...
    Serial.println("Push spice in packet completed");
    Serial.println("===============================");
}
void commandHolderExtend() {
    Serial.println("=== EXECUTING: HOLDER EXTEND ===");
    Serial.println("Extending holder mechanism...");
    // ...логіка...
    Serial.println("Holder extend operation completed");
    Serial.println("===============================");
}
void commandNozzleBack() {
    Serial.println("=== EXECUTING: NOZZLE BACK ===");
    Serial.println("Moving nozzle back...");
    // ...логіка...
    Serial.println("Nozzle back operation completed");
    Serial.println("===============================");
}
void commandVacuumSeal() {
    Serial.println("=== EXECUTING: VACUUM SEAL ===");
    Serial.println("Starting vacuum sealing process...");
    // ...логіка...
    Serial.println("Vacuum seal operation completed");
    Serial.println("===============================");
}
void commandSealerDown() {
    Serial.println("=== EXECUTING: SEALER DOWN ===");
    Serial.println("Lowering sealer mechanism...");
    // ...логіка...
    Serial.println("Sealer down operation completed");
    Serial.println("===============================");
}
void commandHeaterOn() {
    Serial.println("=== EXECUTING: HEATER ON ===");
    Serial.println("Activating heater for sealing...");
    // ...логіка...
    Serial.println("Heater on operation completed");
    Serial.println("===============================");
}
void commandVacuumOff() {
    Serial.println("=== EXECUTING: VACUUM OFF ===");
    Serial.println("Deactivating vacuum system...");
    // ...логіка...
    Serial.println("Vacuum off operation completed");
    Serial.println("===============================");
}
void commandSealerUp() {
    Serial.println("=== EXECUTING: SEALER UP ===");
    Serial.println("Raising sealer mechanism...");
    // ...логіка...
    Serial.println("Sealer up operation completed");
    Serial.println("===============================");
}
void commandCoolerOn() {
    Serial.println("=== EXECUTING: COOLER ON ===");
    Serial.println("Activating cooling system...");
    // ...логіка...
    Serial.println("Cooler on operation completed");
    Serial.println("===============================");
}
void commandHolderUp() {
    Serial.println("=== EXECUTING: HOLDER UP ===");
    Serial.println("Raising holder mechanism...");
    // ...логіка...
    Serial.println("Holder up operation completed");
    Serial.println("===============================");
}
void commandNozzleForward() {
    Serial.println("=== EXECUTING: NOZZLE FORWARD ===");
    Serial.println("Moving nozzle forward...");
    // ...логіка...
    Serial.println("Nozzle forward operation completed");
    Serial.println("===============================");
}
void commandPusherHome() {
    Serial.println("=== EXECUTING: PUSHER HOME ===");
    Serial.println("Returning pusher to home position...");
    // ...логіка...
    Serial.println("Pusher home operation completed");
    Serial.println("===============================");
}
void commandPlatformReturnHome() {
    Serial.println("=== EXECUTING: PLATFORM RETURN HOME ===");
    Serial.println("Returning platform to home position...");
    // ...логіка...
    Serial.println("Platform return home completed");
    Serial.println("===============================");
}
void commandDropPacket() {
    Serial.println("=== EXECUTING: DROP PACKET ===");
    Serial.println("Dropping completed packet...");
    // ...логіка...
    Serial.println("Drop packet operation completed");
    Serial.println("===============================");
}