#include <Arduino.h>
#include "pinout.h"
#include "config.h"
#include "pnevmatik_time.h"
#include "vacuumValve.h"
#include "pneumatic_valve.h"
#include "controls.h"
#include "conveyor.h"


// Ensure DBG_PRINT macros exist (fallback)
#ifndef DBG_PRINT
  #if defined(ENABLE_SERIAL_DEBUG) && (ENABLE_SERIAL_DEBUG == 1)
    #define DBG_PRINT(...)  Serial.print(__VA_ARGS__)
    #define DBG_PRINTLN(...) Serial.println(__VA_ARGS__)
  #else
    #define DBG_PRINT(...)    ((void)0)
    #define DBG_PRINTLN(...)  ((void)0)
  #endif
#endif

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
Conveyor conveyor;      // Основний конвеєр (X+Y)
ConveyorZ conveyorZ;    // Другий конвеєр (Z)

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
unsigned long pauseStartTime = 0; // момент старту паузи

// Неблокуюча пауза між кроками (після виконання команди перед переходом до наступної)
static bool interStepDelayActive = false;
static unsigned long interStepDelayEnd = 0;
static uint8_t interStepWaitingForStep = 255; // номер кроку, після якого чекаємо паузу
static bool interStepWaitMsgPrinted = false;  // чи виведено повідомлення про очікування
static bool singleStepWaitMsgPrinted = false; // чи виведено повідомлення про очікування кнопки START
static uint8_t singleStepWaitStep = 255;      // крок, для якого показано повідомлення очікування

// Таблиця затримок між кроками (мс). За замовчуванням 0. Налаштуйте за потреби.
static const unsigned long stepPauseMs[] = {
  /* 0  commandSpiceOut                  */ STEP_PAUSE_SPICE_OUT_MS,
  /* 1  commandPaintValveOpen            */ STEP_PAUSE_PAINT_VALVE_OPEN_MS,
  /* 2  commandPaintPistonIntake         */ STEP_PAUSE_PAINT_PISTON_INTAKE_MS,
  /* 3  commandPaintValveClose           */ STEP_PAUSE_PAINT_VALVE_CLOSE_MS,
  /* 4  commandPaintPistonDispense       */ STEP_PAUSE_PAINT_PISTON_DISPENSE_MS,
  /* 5  commandCapScrew                  */ STEP_PAUSE_CAP_SCREW_MS,
  /* 6  commandCapClose                  */ STEP_PAUSE_CAP_CLOSE_MS,
  /* 7  commandSpiceShift                */ STEP_PAUSE_SPICE_SHIFT_MS,
  /* 8  commandPlatformHome              */ STEP_PAUSE_PLATFORM_HOME_MS,
  /* 9  commandPlatformDown              */ STEP_PAUSE_PLATFORM_DOWN_MS,
  /* 10 commandVacuumOn                  */ STEP_PAUSE_VACUUM_ON_MS,
  /* 11 commandPlatformUp                */ STEP_PAUSE_PLATFORM_UP_MS,
  /* 12 commandPlatformMoveWithPacket    */ STEP_PAUSE_PLATFORM_MOVE_WITH_PACKET_MS,
  /* 13 commandPlatformDownOverPacket    */ STEP_PAUSE_PLATFORM_DOWN_OVER_PACKET_MS,
  /* 14 commandPlatformUpOpenPacket      */ STEP_PAUSE_PLATFORM_UP_OPEN_PACKET_MS,
  /* 15 commandPushSpiceInPacket         */ STEP_PAUSE_PUSH_SPICE_IN_PACKET_MS,
  /* 16 commandHolderExtend              */ STEP_PAUSE_HOLDER_EXTEND_MS,
  /* 17 commandNozzleBack                */ STEP_PAUSE_NOZZLE_BACK_MS,
  /* 18 commandVacuumSeal                */ STEP_PAUSE_VACUUM_SEAL_MS,
  /* 19 commandSealerDown                */ STEP_PAUSE_SEALER_DOWN_MS,
  /* 20 commandHeaterOn                  */ STEP_PAUSE_HEATER_ON_MS,
  /* 21 commandVacuumOff                 */ STEP_PAUSE_VACUUM_OFF_MS,
  /* 22 commandSealerUp                  */ STEP_PAUSE_SEALER_UP_MS,
  /* 23 commandCoolerOn                  */ STEP_PAUSE_COOLER_ON_MS,
  /* 24 commandHolderUp                  */ STEP_PAUSE_HOLDER_UP_MS,
  /* 25 commandNozzleForward             */ STEP_PAUSE_NOZZLE_FORWARD_MS,
  /* 26 commandPusherHome                */ STEP_PAUSE_PUSHER_HOME_MS,
  /* 27 commandPlatformReturnHome        */ STEP_PAUSE_PLATFORM_RETURN_HOME_MS,
  /* 28 commandDropPacket                */ STEP_PAUSE_DROP_PACKET_MS
};

static inline unsigned long getInterStepDelayMs(uint8_t step) {
    if (step < (sizeof(stepPauseMs) / sizeof(stepPauseMs[0]))) return stepPauseMs[step];
    return 0;
}

static inline void startInterStepDelay(uint8_t step) {
    interStepDelayActive = true;
    interStepDelayEnd = millis() + getInterStepDelayMs(step);
    interStepWaitingForStep = step;
    interStepWaitMsgPrinted = false;
}

// Змінні для логіки роботи згідно алгоритму
uint8_t jarCounter = 0;           // Лічильник баночок (0-5)
uint8_t jarsSeenInSet = 0;        // К-сть баночок з набору, що пройшли під датчиком 1 (0..6)
uint8_t spiceSetCounter = 0;      // Лічильник спайок (0-3, потрібно 4)
bool waitingForSensor1 = false;   // Очікування спрацювання датчика 1
bool waitingForSensor2 = false;   // Очікування спрацювання датчика 2
bool paintCycleActive = false;    // Активний цикл розливу фарби
bool capCycleActive = false;      // Активний цикл закривання кришок
bool packagingCycleActive = false; // Активний цикл пакування

// Таймери для датчиків та циклів
unsigned long sensor1StartTime = 0;    // Час початку очікування датчика 1
unsigned long sensor2StartTime = 0;    // Час початку очікування датчика 2
unsigned long paintCycleStartTime = 0; // Час початку циклу розливу фарби
unsigned long capCycleStartTime = 0;   // Час початку циклу закривання кришок
unsigned long packagingCycleStartTime = 0; // Час початку циклу пакування

// Змінні для відладки
unsigned long lastDebugTime = 0;
const unsigned long DEBUG_INTERVAL = 4000; // 4 секунди між оновленнями відладки

void setup() {
  Serial.begin(9600);
  Serial.println("=== Machine Startup ===");
  
  // Heater initialization
  pinMode(heaterPin, OUTPUT);
  digitalWrite(heaterPin, LOW);
  
  // Sensor initialization (via Controls class)
  
  // Vacuum valve initialization
  vacuumValveInit();
  setVacuumValve(VALVE_POS_1);

  // Pneumatic valve initialization
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
  
  // Controls and conveyor initialization
  ControlsConfig cfg; // за замовчуванням: кнопки моментні, датчики без інверсії
  controls.begin(cfg);
  conveyor.begin();
  conveyorZ.begin();
  
  // Sensor testing
/* Serial.println("Testing sensors...");
  SensorUtils::testAllSensors(
    []() -> bool { return controls.isSensor1Active(); },
    []() -> bool { return controls.isSensor2Active(); },
    []() -> bool { return controls.isSensor3Active(); }
  );
  */
  // Initial valve positions
  //valve1.on();
  //valve3.on();
  
//  Serial.println("=== SETUP COMPLETED ===");
  Serial.println("System ready for operation!");
  Serial.println("=====================================");
}

void loop() {
    // Оновлення стану кнопок і датчиків
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
    conveyor.update();
    conveyorZ.update();
    // Отримуємо стан датчиків (через клас Controls)
    bool sensor1 = controls.isSensor1Active();
    bool sensor2 = controls.isSensor2Active();
    bool sensor3 = controls.isSensor3Active();

    // --- SENSOR LOGIC ACCORDING TO ALGORITHM ---
    
    // Sensor 1: Jar under paint dispensing nozzle (edge-based, with mode logic)
    static bool s1Prev = false;
    bool s1Rise = (!s1Prev && sensor1);
    s1Prev = sensor1;
    if (s1Rise && !waitingForSensor1 && machineRunning && !machinePaused) {
        jarsSeenInSet++;
        if (!dispenseMode && jarsSeenInSet > 1) {
            Serial.println("S1: ONE-JAR mode, ignoring jar > 1 in set");
        } else {
            waitingForSensor1 = true;
            sensor1StartTime = millis();
            Serial.println("=== SENSOR 1: Jar under paint dispensing nozzle ===");
            conveyor.stopWithDociag(JAR_CENTERING_MM);
            Serial.print("Conveyor stopping with overrun of ");
            Serial.print(JAR_CENTERING_MM);
            Serial.println(" mm for jar centering");
        }
    }
    
    // Sensor 2: Jar under cap closing press (trigger on rising edge to avoid retrigger while jar stays under press)
    bool s2Rise = controls.sensor2RisingEdge();
    if (s2Rise && !waitingForSensor2 && machineRunning && !machinePaused) {
        waitingForSensor2 = true;
        sensor2StartTime = millis();
        Serial.println("=== SENSOR 2: Jar under cap closing press ===");
        
        // Stop conveyor immediately
        conveyor.stop();
        Serial.println("Conveyor stopped");
        
        // Start cap closing cycle
        capCycleActive = true;
        capCycleStartTime = millis();
        currentStep = 5; // Start with cap screwing command
        Serial.println("Cap closing cycle started");
    }
    
    // Sensor 3: Spice set ready for shifting (тільки після закривання кришок)
    // Цей датчик не обробляється окремо - він спрацьовує в циклі закривання кришок

    // Отримуємо стан кнопок
    bool startBtn = controls.startPressed();
    bool stopBtn = controls.stopPressed();
    bool modeBtn = controls.modeChanged();
    bool singleBtn = controls.singleBlockPressed();

        // --- START Button ---
    if (startBtn) {
        if (singleBlockMode) {
            // У режимі одиночного кроку: виконуємо наступну команду лише якщо пауза не активна
            if (!interStepDelayActive && currentStep < machineProgramLength) {
                Serial.println("=== START (SINGLE STEP) ===");
                Serial.print("Next step: ");
                Serial.println(currentStep);

                machineProgram[currentStep]();
                startInterStepDelay(currentStep);
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
            // Якщо пауза активна — резюме без скидання станів і таймерів
            if (machinePaused) {
                unsigned long delta = millis() - pauseStartTime;
                machinePaused = false;
                machineRunning = true;
                conveyor.start();
                conveyorZ.start();

                // Зсунути таймери клапанів
                valve1.shiftTimers(delta);  valve2.shiftTimers(delta);  valve3.shiftTimers(delta);
                valve4.shiftTimers(delta);  valve5.shiftTimers(delta);  valve6.shiftTimers(delta);
                valve7.shiftTimers(delta);  valve8.shiftTimers(delta);  valve9.shiftTimers(delta);
                valve10.shiftTimers(delta); valve11.shiftTimers(delta); valve12.shiftTimers(delta);
                valve13.shiftTimers(delta); valve14.shiftTimers(delta);

                // Зсунути алгоритмічні таймери, якщо вони були активні (ненульові)
                if (sensor1StartTime)       sensor1StartTime       += delta;
                if (sensor2StartTime)       sensor2StartTime       += delta;
                if (paintCycleStartTime)    paintCycleStartTime    += delta;
                if (capCycleStartTime)      capCycleStartTime      += delta;
                if (packagingCycleStartTime)packagingCycleStartTime+= delta;
                if (interStepDelayActive)   interStepDelayEnd      += delta;

                Serial.println("=== RESUME ===");
                Serial.println("Resuming machine without state reset");
            } else {
                // Перший старт — ініціалізація циклу
                machinePaused = false;
                conveyor.start();
                conveyorZ.start();
                machineRunning = true;

                // Скидання лічильників та станів лише при першому старті
                jarCounter = 0;
                spiceSetCounter = 0;
                waitingForSensor1 = false;
                waitingForSensor2 = false;
                paintCycleActive = false;
                capCycleActive = false;
                packagingCycleActive = false;
                currentStep = 0;

                // Скидання таймерів
                sensor1StartTime = 0;
                sensor2StartTime = 0;
                paintCycleStartTime = 0;
                capCycleStartTime = 0;
                packagingCycleStartTime = 0;

                Serial.println("=== START PRESSED ===");
                Serial.println("Machine started - OPERATION mode");
                Serial.println("Conveyors enabled and started");
                Serial.println("Counters and states initialized");
            }
        }
    }
    
    // --- STOP Button ---
    if (stopBtn) {
        // Пауза: зупинити обидва конвеєри та запам'ятати момент паузи, без скидань станів
        if (!machinePaused) {
            machinePaused = true;
            pauseStartTime = millis();
            conveyor.stop();
            conveyorZ.stop();
            machineRunning = false;
            Serial.println("=== STOP (PAUSE) PRESSED ===");
            Serial.println("Machine paused. States preserved, timers will be shifted on resume");
        }
    }
    
    // --- Single Block Button ---
    if (singleBtn) {
        singleBlockMode = !singleBlockMode;
        currentStep = 0;
        machineRunning = false;
        
        // Reset all cycle states
        paintCycleActive = false;
        capCycleActive = false;
        packagingCycleActive = false;
        waitingForSensor1 = false;
        waitingForSensor2 = false;
        
        // Reset all timers
        sensor1StartTime = 0;
        sensor2StartTime = 0;
        paintCycleStartTime = 0;
        capCycleStartTime = 0;
        packagingCycleStartTime = 0;
        
        Serial.println("=== SINGLE STEP BUTTON PRESSED ===");
        Serial.print("Single step mode: ");
        Serial.println(singleBlockMode ? "ENABLED" : "DISABLED");
        Serial.println("Current step reset to 0");
        Serial.println("All cycle states reset");
        
        if (singleBlockMode) {
            Serial.println("Press START to execute next step");
            Serial.println("Press STOP to pause execution");
        }
    }
    
    // --- Mode switch handling ---
    // If mode button configured as TOGGLE (two-position switch), map its level to dispenseMode
    if (controls.isModeToggleConfigured()) {
        bool desired = controls.modeToggle();
        if (desired != dispenseMode) {
            dispenseMode = desired;
            Serial.println("=== DISPENSE MODE SWITCH CHANGED ===");
            Serial.println(dispenseMode ? "Mode: ALL JARS" : "Mode: ONE JAR");
            jarCounter = 0;
            jarsSeenInSet = 0;
        }
    } else if (modeBtn) {
        // Momentary button: toggle mode on press
        dispenseMode = !dispenseMode;
        Serial.println("=== DISPENSE MODE BUTTON PRESSED ===");
        Serial.println(dispenseMode ? "Mode: ALL JARS" : "Mode: ONE JAR");
        jarCounter = 0;
        jarsSeenInSet = 0;
    }
    
    /*// --- Valve testing (demonstrating onFor/offFor) ---
    static unsigned long lastValveTest = 0;
    static bool valveTestState = false;
    
    if (millis() - lastValveTest > 5000) { // Every 5 seconds
        lastValveTest = millis();
        valveTestState = !valveTestState;
        
        if (valveTestState) {
            Serial.println("=== VALVE TEST: onFor(1000ms) ===");
            valve1.onFor(1000); // Turn on for 1 second
            Serial.println("Valve 1 will automatically turn OFF in 1 second");
        } else {
            Serial.println("=== VALVE TEST: offFor(1000ms) ===");
            valve1.offFor(1000); // Turn off for 1 second
            Serial.println("Valve 1 will automatically turn ON in 1 second");
        }
    }
    */
    // --- Sensor debugging (every 2 seconds) ---
    unsigned long now = millis();
    if (now - lastDebugTime > DEBUG_INTERVAL) {
        lastDebugTime = now;

        if (DEBUG_LEVEL == 0) {
            // Output nothing
        } else if (DEBUG_LEVEL == 1) {
            // Basic information
            Serial.println("=== STATUS (BASIC) ===");
            Serial.print("Machine status: ");
            if (machinePaused) {
                Serial.println("PAUSED");
            } else if (machineRunning) {
                Serial.println("RUNNING");
            } else {
                Serial.println("STOPPED");
            }

            Serial.print("Operation mode: ");
            Serial.println(singleBlockMode ? "SINGLE STEP" : "AUTO");

            Serial.print("Dispense mode: ");
            Serial.println(dispenseMode ? "ALL JARS" : "ONE JAR");

                    Serial.print("Current step: ");
        Serial.print(currentStep);
        Serial.print("/");
        Serial.println(machineProgramLength);
        
        Serial.print("Jar counter: ");
        Serial.print(jarCounter);
        Serial.print("/");
        Serial.println(JARS_IN_SET);
        
        Serial.print("Spice set counter: ");
        Serial.print(spiceSetCounter);
        Serial.print("/");
        Serial.println(SPICE_SETS_PER_PACKAGE);
        
        Serial.print("Conveyor status: ");
        Serial.println(conveyor.isRunning() ? "RUNNING" : "STOPPED");
        
        Serial.print("Active cycles: ");
        if (paintCycleActive) Serial.print("PAINT ");
        if (capCycleActive) Serial.print("CAPS ");
        if (packagingCycleActive) Serial.print("PACKAGING ");
        if (!paintCycleActive && !capCycleActive && !packagingCycleActive) Serial.print("NONE");
        Serial.println();
        
        // Timer information
        if (paintCycleActive) {
            Serial.print("Paint cycle time: ");
            Serial.print(MS_TO_SECONDS(millis() - paintCycleStartTime));
            Serial.print("/");
            Serial.print(MS_TO_SECONDS(PAINT_CYCLE_TOTAL_TIME));
            Serial.println(" seconds");
        }
        
        if (capCycleActive) {
            Serial.print("Cap cycle time: ");
            Serial.print(MS_TO_SECONDS(millis() - capCycleStartTime));
            Serial.print("/");
            Serial.print(MS_TO_SECONDS(CAP_CYCLE_TOTAL_TIME));
            Serial.println(" seconds");
        }
        
        if (packagingCycleActive) {
            Serial.print("Packaging cycle time: ");
            Serial.print(MS_TO_SECONDS(millis() - packagingCycleStartTime));
            Serial.print("/");
            Serial.print(MS_TO_SECONDS(PACKAGING_CYCLE_TOTAL_TIME));
            Serial.println(" seconds");
        }
        
        Serial.println("=======================");
        } else { // DEBUG_LEVEL >= 2 — full detailed debugging
            // Output detailed information to Serial
            Serial.println("=== STATUS UPDATE ===");
            Serial.print("Machine status: ");
            if (machinePaused) {
                Serial.println("PAUSED");
            } else if (machineRunning) {
                Serial.println("RUNNING");
            } else {
                Serial.println("STOPPED");
            }
            
            Serial.print("Operation mode: ");
            Serial.println(singleBlockMode ? "SINGLE STEP" : "AUTO");
            
            Serial.print("Dispense mode: ");
            Serial.println(dispenseMode ? "ALL JARS" : "ONE JAR");
            
                    Serial.print("Current step: ");
        Serial.print(currentStep);
        Serial.print("/");
        Serial.println(machineProgramLength);
        
        if (currentStep < machineProgramLength) {
            Serial.print("Next command: ");
            Serial.println(currentStep == 0 ? "SPICE OUTPUT" : 
                         currentStep == 1 ? "PAINT VALVE OPEN" :
                         currentStep == 2 ? "PISTON INTAKE" :
                         currentStep == 3 ? "PAINT VALVE CLOSE" :
                         currentStep == 4 ? "PISTON DISPENSE" :
                         currentStep == 5 ? "CAP SCREWING" :
                         currentStep == 6 ? "CAP CLOSING" :
                         currentStep == 7 ? "SPICE SHIFT" :
                         "OTHER COMMAND");
        }
        
        Serial.print("Jar counter: ");
        Serial.print(jarCounter);
        Serial.print("/");
        Serial.println(JARS_IN_SET);
        
        Serial.print("Spice set counter: ");
        Serial.print(spiceSetCounter);
        Serial.print("/");
        Serial.println(SPICE_SETS_PER_PACKAGE);
        
        Serial.print("Conveyor status: ");
        Serial.println(conveyor.isRunning() ? "RUNNING" : "STOPPED");
        
        Serial.print("Conveyor overrun: ");
        Serial.println(conveyor.isDociagActive() ? "ACTIVE" : "INACTIVE");
        
        Serial.println("--- SENSOR STATUS ---");
        Serial.print("Sensor 1 (Paint dispensing): ");
        Serial.println(sensor1 ? "ACTIVE - Jar under nozzle" : "INACTIVE - No jar");
        
        Serial.print("Sensor 2 (Cap press): ");
        Serial.println(sensor2 ? "ACTIVE - Jar under press" : "INACTIVE - No jar");
        
        Serial.print("Sensor 3 (Spice shift): ");
        Serial.println(sensor3 ? "ACTIVE - Spice set ready" : "INACTIVE - Not ready");
        
        Serial.println("--- BUTTON STATUS ---");
        Serial.print("START button: ");
        Serial.println(startBtn ? "PRESSED" : "RELEASED");
        
        Serial.print("STOP button: ");
        Serial.println(stopBtn ? "PRESSED" : "RELEASED");
        
        Serial.print("MODE button: ");
        Serial.println(modeBtn ? "PRESSED" : "RELEASED");
        
        Serial.print("SINGLE STEP button: ");
        Serial.println(singleBtn ? "PRESSED" : "RELEASED");
        
        Serial.println("--- VALVE STATUS ---");
        Serial.print("Valve 1: ");
        Serial.println(valve1.isOn() ? "ON" : "OFF");
        Serial.print("Valve 2: ");
        Serial.println(valve2.isOn() ? "ON" : "OFF");
        Serial.print("Valve 3: ");
        Serial.println(valve3.isOn() ? "ON" : "OFF");
        
        Serial.println("--- CYCLE STATUS ---");
        Serial.print("Paint cycle: ");
        Serial.println(paintCycleActive ? "ACTIVE" : "INACTIVE");
        Serial.print("Cap closing cycle: ");
        Serial.println(capCycleActive ? "ACTIVE" : "INACTIVE");
        Serial.print("Packaging cycle: ");
        Serial.println(packagingCycleActive ? "ACTIVE" : "INACTIVE");
        
        // Timer information for detailed debug
        Serial.println("--- TIMER STATUS ---");
        if (waitingForSensor1) {
            Serial.print("Sensor 1 wait time: ");
            Serial.print(MS_TO_SECONDS(millis() - sensor1StartTime));
            Serial.print("/");
            Serial.print(MS_TO_SECONDS(SENSOR_TIMEOUT_TIME));
            Serial.println(" seconds");
        }
        
        if (waitingForSensor2) {
            Serial.print("Sensor 2 wait time: ");
            Serial.print(MS_TO_SECONDS(millis() - sensor2StartTime));
            Serial.print("/");
            Serial.print(MS_TO_SECONDS(SENSOR_TIMEOUT_TIME));
            Serial.println(" seconds");
        }
        

        
        if (paintCycleActive) {
            Serial.print("Paint cycle time: ");
            Serial.print(MS_TO_SECONDS(millis() - paintCycleStartTime));
            Serial.print("/");
            Serial.print(MS_TO_SECONDS(PAINT_CYCLE_TOTAL_TIME));
            Serial.println(" seconds");
        }
        
        if (capCycleActive) {
            Serial.print("Cap cycle time: ");
            Serial.print(MS_TO_SECONDS(millis() - capCycleStartTime));
            Serial.print("/");
            Serial.print(MS_TO_SECONDS(CAP_CYCLE_TOTAL_TIME));
            Serial.println(" seconds");
        }
        
        if (packagingCycleActive) {
            Serial.print("Packaging cycle time: ");
            Serial.print(MS_TO_SECONDS(millis() - packagingCycleStartTime));
            Serial.print("/");
            Serial.print(MS_TO_SECONDS(PACKAGING_CYCLE_TOTAL_TIME));
            Serial.println(" seconds");
        }
        
        Serial.println("===================");
        }
    }

    // --- Mechanism updates only if not paused ---
    if (!machinePaused) {
        // Update conveyor always (it controls its own state)
        //conveyor.update();
        
        // Additional debugging for understanding state
        static unsigned long lastStatusCheck = 0;
        if (millis() - lastStatusCheck > 1000) { // Every second
            lastStatusCheck = millis();
            
            if (singleBlockMode) {
                Serial.print("SINGLE STEP: Waiting for START button. Step: ");
                Serial.println(currentStep);
            } else if (machineRunning) {
                if (paintCycleActive) {
                    Serial.print("PAINT CYCLE: Step ");
                    Serial.println(currentStep);
                } else if (capCycleActive) {
                    Serial.print("CAP CLOSING CYCLE: Step ");
                    Serial.println(currentStep);
                } else if (packagingCycleActive) {
                    Serial.print("PACKAGING CYCLE: Step ");
                    Serial.println(currentStep);
                } else {
                    Serial.println("AUTO MODE: Waiting for sensor triggers");
                }
            } else {
                Serial.println("Machine idle - press START or enable SINGLE STEP mode");
            }
        }
    }

    // --- COMMAND EXECUTION LOGIC ACCORDING TO ALGORITHM ---
    if (!machinePaused) {
        if (singleBlockMode) {
            // Режим одиночного кроку: виконання відбувається ТІЛЬКИ по натисканню START (у обробнику кнопки)
            // Тут лише інформуємо про стан очікування/паузи
            if (interStepDelayActive) {
                if (!interStepWaitMsgPrinted) {
                    interStepWaitMsgPrinted = true;
                    Serial.print("Inter-step delay active (step ");
                    Serial.print(interStepWaitingForStep);
                    Serial.print(") remaining ~");
                    {
                        long remain = (long)(interStepDelayEnd - millis());
                        if (remain < 0) remain = 0;
                        Serial.print(remain);
                    }
                    Serial.println(" ms");
                }
                if (millis() >= interStepDelayEnd) {
                    interStepDelayActive = false;
                    Serial.println("Inter-step delay finished");
                }
            } else {
                if (!singleStepWaitMsgPrinted || singleStepWaitStep != currentStep) {
                    singleStepWaitMsgPrinted = true;
                    singleStepWaitStep = currentStep;
                    Serial.print("SINGLE STEP: Waiting for START button. Step: ");
                    Serial.println(currentStep);
                }
            }
        } else if (machineRunning) {
            // Automatic mode - execute commands according to active cycles
            
            // Check if overrun completed for sensor 1
            if (waitingForSensor1 && !conveyor.isRunning() && !paintCycleActive) {
                // Overrun completed, start paint cycle
                paintCycleActive = true;
                paintCycleStartTime = millis();
                currentStep = 1;
                Serial.println("Overrun completed, paint cycle started");
            }
            
            // Check for sensor timeouts
            if (waitingForSensor1 && IS_TIMEOUT(sensor1StartTime, SENSOR_TIMEOUT_TIME)) {
                Serial.println("WARNING: Sensor 1 timeout - restarting conveyor");
                waitingForSensor1 = false;
                conveyor.start();
            }
            
            if (waitingForSensor2 && IS_TIMEOUT(sensor2StartTime, SENSOR_TIMEOUT_TIME)) {
                Serial.println("WARNING: Sensor 2 timeout - restarting conveyor");
                waitingForSensor2 = false;
                capCycleActive = false;
                conveyor.start();
            }
            
            // Paint dispensing cycle (steps 1-4)
            if (paintCycleActive && currentStep >= 1 && currentStep <= 4) {
                if (!interStepDelayActive) {
                    Serial.print("=== PAINT CYCLE: Step ");
                    Serial.print(currentStep);
                    Serial.println(" ===");
                    machineProgram[currentStep]();
                    startInterStepDelay(currentStep);
                    currentStep++;
                } else if (millis() >= interStepDelayEnd) {
                    interStepDelayActive = false;
                } else if (!interStepWaitMsgPrinted) {
                    interStepWaitMsgPrinted = true;
                    Serial.print("PAINT: inter-step delay active (step ");
                    Serial.print(interStepWaitingForStep);
                    Serial.print(") remaining ~");
                    Serial.print((long)(interStepDelayEnd - millis()));
                    Serial.println(" ms");
                }
                
                // Check if paint cycle completed
                if (currentStep > 4) {
                    paintCycleActive = false;
                    waitingForSensor1 = false;
                    jarCounter++;
                    Serial.print("Paint cycle completed. Jar ");
                    Serial.print(jarCounter);
                    Serial.println(" processed");

                    if (dispenseMode) {
                        if (jarCounter >= JARS_IN_SET) {
                            jarCounter = 0;
                            jarsSeenInSet = 0;
                            Serial.println("All jars in set processed");
                        }
                        conveyor.start();
                        Serial.println("Conveyor started for next jar");
                    } else {
                        // ONE-JAR mode: only first jar dispensed, keep conveyor running continuously
                        if (jarsSeenInSet >= JARS_IN_SET) {
                            jarsSeenInSet = 0;
                            jarCounter = 0;
                            Serial.println("ONE-JAR mode: set completed");
                        }
                        conveyor.start();
                    }
                }
                
                // Check for paint cycle timeout
                if (paintCycleActive && IS_TIMEOUT(paintCycleStartTime, PAINT_CYCLE_TOTAL_TIME)) {
                    Serial.println("WARNING: Paint cycle timeout - restarting conveyor");
                    paintCycleActive = false;
                    waitingForSensor1 = false;
                    conveyor.start();
                }
            }
            
            // Cap closing cycle (steps 5-6)
            else if (capCycleActive && currentStep >= 5 && currentStep <= 6) {
                if (!interStepDelayActive) {
                    Serial.print("=== CAP CLOSING CYCLE: Step ");
                    Serial.print(currentStep);
                    Serial.println(" ===");
                    machineProgram[currentStep]();
                    startInterStepDelay(currentStep);
                    currentStep++;
                } else if (millis() >= interStepDelayEnd) {
                    interStepDelayActive = false;
                } else if (!interStepWaitMsgPrinted) {
                    interStepWaitMsgPrinted = true;
                    Serial.print("CAP: inter-step delay active (step ");
                    Serial.print(interStepWaitingForStep);
                    Serial.print(") remaining ~");
                    Serial.print((long)(interStepDelayEnd - millis()));
                    Serial.println(" ms");
                }
                
                // Check if cap closing cycle completed
                if (currentStep > 6) {
                    capCycleActive = false;
                    waitingForSensor2 = false;
                    
                    Serial.println("Cap closing cycle completed");
                    
                    // Після закривання кришок перевіряємо третій датчик
                    if (sensor3) {
                        // Increment spice set counter
                        spiceSetCounter++;
                        Serial.print("Spice set counter: ");
                        Serial.print(spiceSetCounter);
                        Serial.println("/4");
                        
                        // If collected required spice sets - start packaging cycle
                        if (spiceSetCounter >= SPICE_SETS_PER_PACKAGE) {
                            packagingCycleActive = true;
                            packagingCycleStartTime = millis();
                            currentStep = 7; // Start with spice shift command
                            Serial.println("Packaging cycle started (required spice sets ready)");
                            // Зупинити другий конвеєр на час пакування
                            conveyorZ.stop();
                        } else {
                            // Otherwise just shift spice set and continue
                            commandSpiceShift();
                            conveyor.start();
                            // Другий конвеєр може продовжувати рух незалежно
                            Serial.println("Spice set shifted, conveyor continues operation");
                        }
                    } else {
                        // No spice set ready, just continue
                        conveyor.start();
                        Serial.println("Conveyor started for next operation");
                    }
                }
                
                // Check for cap cycle timeout
                if (capCycleActive && IS_TIMEOUT(capCycleStartTime, CAP_CYCLE_TOTAL_TIME)) {
                    Serial.println("WARNING: Cap closing cycle timeout - restarting conveyor");
                    capCycleActive = false;
                    waitingForSensor2 = false;
                    conveyor.start();
                }
            }
            
            // Packaging cycle (steps 7-29)
            else if (packagingCycleActive && currentStep >= 7 && currentStep < machineProgramLength) {
                if (!interStepDelayActive) {
                    Serial.print("=== PACKAGING CYCLE: Step ");
                    Serial.print(currentStep);
                    Serial.println(" ===");
                    machineProgram[currentStep]();
                    startInterStepDelay(currentStep);
                    currentStep++;
                } else if (millis() >= interStepDelayEnd) {
                    interStepDelayActive = false;
                } else if (!interStepWaitMsgPrinted) {
                    interStepWaitMsgPrinted = true;
                    Serial.print("PACKAGING: inter-step delay active (step ");
                    Serial.print(interStepWaitingForStep);
                    Serial.print(") remaining ~");
                    Serial.print((long)(interStepDelayEnd - millis()));
                    Serial.println(" ms");
                }
                
                // Check if packaging cycle completed
                if (currentStep >= machineProgramLength) {
                    packagingCycleActive = false;
                    spiceSetCounter = 0;
                    
                    Serial.println("Packaging cycle completed");
                    conveyor.start();
                    conveyorZ.start();
                    Serial.println("Conveyors started for next set");
                    
                    // Reset step for next cycle
                    currentStep = 0;
                }
                
                // Check for packaging cycle timeout
                if (packagingCycleActive && IS_TIMEOUT(packagingCycleStartTime, PACKAGING_CYCLE_TOTAL_TIME)) {
                    Serial.println("WARNING: Packaging cycle timeout - restarting conveyor");
                    packagingCycleActive = false;
                    spiceSetCounter = 0;
                    conveyor.start();
                }
            }
        }
    }
}

// Додаткова логіка керування другим конвеєром по датчику 3 (центрування спайки)
// Обробляємо події фронту для sensor3 окремо на початку loop

// --- Helper Functions ---

void executeStep(uint8_t step) {
    // Function for manual command execution (can be used for testing)
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

// Removed unused toggleDispenseMode()

void commandSpiceOut() {
    Serial.println("=== SPICE OUTPUT ===");
    Serial.print("Activating spice output valve for ");
    Serial.print(MS_TO_SECONDS(SPICE_OUT_HOLD_TIME));
    Serial.println(" seconds");
    valve1.onFor(SPICE_OUT_HOLD_TIME); // Distributor #1 - spice output
    Serial.println("SPICE OUTPUT COMPLETED");
}

void commandPaintValveOpen() {
    Serial.println("=== PAINT VALVE OPEN ===");
    bool sensor1 = controls.isSensor1Active();
    Serial.print("Sensor 1 (jar under nozzle): ");
    Serial.println(sensor1 ? "ACTIVE" : "INACTIVE");
    
    if (sensor1) {
        Serial.print("Opening main paint reservoir valve for ");
        Serial.print(MS_TO_SECONDS(PAINT_VALVE_HOLD_TIME));
        Serial.println(" seconds");
        valve2.onFor(PAINT_VALVE_HOLD_TIME); // Distributor #2 - reservoir valve open
        Serial.println("PAINT VALVE OPENED");
    } else {
        Serial.println("ERROR: No jar under nozzle!");
    }
}

void commandPaintPistonIntake() {
    Serial.println("=== PAINT PISTON INTAKE ===");
    bool sensor1 = controls.isSensor1Active();
    Serial.print("Sensor 1 (jar under nozzle): ");
    Serial.println(sensor1 ? "ACTIVE" : "INACTIVE");
    
    if (sensor1) {
        Serial.print("Piston moving to paint intake position for ");
        Serial.print(MS_TO_SECONDS(PAINT_PISTON_IN_HOLD_TIME));
        Serial.println(" seconds");
        valve3.onFor(PAINT_PISTON_IN_HOLD_TIME); // Distributor #3 - piston movement for paint intake
        Serial.println("PAINT PISTON INTAKE COMPLETED");
    } else {
        Serial.println("ERROR: No jar under nozzle!");
    }
}

void commandPaintValveClose() {
    Serial.println("=== PAINT VALVE CLOSE ===");
    Serial.println("Closing main paint reservoir valve");
    valve2.off(); // Distributor #2 - reservoir valve close
    Serial.println("PAINT VALVE CLOSED");
}

void commandPaintPistonDispense() {
    Serial.println("=== PAINT PISTON DISPENSE ===");
    bool sensor1 = controls.isSensor1Active();
    Serial.print("Sensor 1 (jar under nozzle): ");
    Serial.println(sensor1 ? "ACTIVE" : "INACTIVE");
    
    if (sensor1) {
        Serial.print("Piston dispensing paint into jar for ");
        Serial.print(MS_TO_SECONDS(PAINT_PISTON_OUT_HOLD_TIME));
        Serial.println(" seconds");
        valve3.offFor(PAINT_PISTON_OUT_HOLD_TIME); // Distributor #3 - piston movement for paint dispensing
        Serial.println("PAINT PISTON DISPENSE COMPLETED");
    } else {
        Serial.println("ERROR: No jar under nozzle!");
    }
}

void commandCapScrew() {
    Serial.println("=== CAP SCREWING ===");
    bool sensor2 = controls.isSensor2Active();
    Serial.print("Sensor 2 (jar under press): ");
    Serial.println(sensor2 ? "ACTIVE" : "INACTIVE");
    
    if (sensor2) {
        Serial.print("Screwing jar caps before closing for ");
        Serial.print(MS_TO_SECONDS(TWIST_CAP_HOLD_TIME));
        Serial.println(" seconds");
        valve4.onFor(TWIST_CAP_HOLD_TIME); // Distributor #4 - cap screwing
        Serial.println("CAP SCREWING COMPLETED");
    } else {
        Serial.println("ERROR: No jar under press!");
    }
}

void commandCapClose() {
    Serial.println("=== CAP CLOSING ===");
    bool sensor2 = controls.isSensor2Active();
    Serial.print("Sensor 2 (jar under press): ");
    Serial.println(sensor2 ? "ACTIVE" : "INACTIVE");
    
    if (sensor2) {
        Serial.print("Closing jar caps for ");
        Serial.print(MS_TO_SECONDS(CLOSE_CAP_HOLD_TIME));
        Serial.println(" seconds");
        valve5.onFor(CLOSE_CAP_HOLD_TIME); // Distributor #5 - cap closing
        Serial.println("CAP CLOSING COMPLETED");
    } else {
        Serial.println("ERROR: No jar under press!");
    }
}

void commandSpiceShift() {
    Serial.println("=== SPICE SHIFT ===");
    Serial.print("Shifting spice set to packaging position for ");
    Serial.print(MS_TO_SECONDS(SPICE_SHIFT_HOLD_TIME));
    Serial.println(" seconds");
    valve6.onFor(SPICE_SHIFT_HOLD_TIME); // Distributor #6 - spice shift for packaging
    Serial.println("SPICE SHIFT COMPLETED");
}

void commandPlatformHome() {
    Serial.println("=== PLATFORM HOME POSITION ===");
    Serial.print("Moving suction platform to home position for ");
    Serial.print(MS_TO_SECONDS(PLATFORM_MOVE_HOLD_TIME));
    Serial.println(" seconds");
    valve7.onFor(PLATFORM_MOVE_HOLD_TIME); // Distributor #7 - suction platform movement
    Serial.println("PLATFORM IN HOME POSITION");
}

void commandPlatformDown() {
    Serial.println("=== PLATFORM DOWN ===");
    Serial.print("Lowering suction platform for ");
    Serial.print(MS_TO_SECONDS(PLATFORM_LIFT_HOLD_TIME));
    Serial.println(" seconds");
    valve8.onFor(PLATFORM_LIFT_HOLD_TIME); // Distributor #8 - platform down/up movement
    Serial.println("PLATFORM LOWERED");
}

void commandVacuumOn() {
    Serial.println("=== VACUUM ON ===");
    Serial.print("Applying vacuum to suction cups for packet capture for ");
    Serial.print(MS_TO_SECONDS(VACUUM_HOLD_TIME));
    Serial.println(" seconds");
    
    // Тут буде логіка керування вакуумом через серво
    // Поки що використовуємо таймер для симуляції
    Serial.println("VACUUM COMPLETED");
}

void commandPlatformUp() {
    Serial.println("=== PLATFORM UP ===");
    Serial.print("Raising suction platform for ");
    Serial.print(MS_TO_SECONDS(PLATFORM_LIFT_HOLD_TIME));
    Serial.println(" seconds");
    valve8.offFor(PLATFORM_LIFT_HOLD_TIME); // Distributor #8 - platform raise
    Serial.println("PLATFORM RAISED");
}

void commandPlatformMoveWithPacket() {
    Serial.println("=== PLATFORM MOVE WITH PACKET ===");
    Serial.print("Moving suction platform with packet for ");
    Serial.print(MS_TO_SECONDS(PLATFORM_MOVE_HOLD_TIME));
    Serial.println(" seconds");
    valve7.offFor(PLATFORM_MOVE_HOLD_TIME); // Distributor #7 - platform movement with packet
    Serial.println("PLATFORM MOVED WITH PACKET");
}

void commandPlatformDownOverPacket() {
    Serial.println("=== PLATFORM DOWN OVER PACKET ===");
    Serial.print("Lowering platform over closed packet for ");
    Serial.print(MS_TO_SECONDS(PLATFORM_LIFT_HOLD_TIME));
    Serial.println(" seconds");
    valve8.onFor(PLATFORM_LIFT_HOLD_TIME); // Distributor #8 - platform down
    Serial.println("PLATFORM LOWERED OVER PACKET");
}

void commandPlatformUpOpenPacket() {
    Serial.println("=== PLATFORM UP AND PACKET OPEN ===");
    Serial.print("Raising platform and opening packet for ");
    Serial.print(MS_TO_SECONDS(PLATFORM_LIFT_HOLD_TIME));
    Serial.println(" seconds");
    valve8.offFor(PLATFORM_LIFT_HOLD_TIME); // Distributor #8 - platform raise
    Serial.println("PLATFORM RAISED, PACKET OPENED");
}

void commandPushSpiceInPacket() {
    Serial.println("=== PUSH SPICE INTO PACKET ===");
    Serial.print("Pushing spice set from platform into packet for ");
    Serial.print(MS_TO_SECONDS(SPICE_PUSH_HOLD_TIME));
    Serial.println(" seconds");
    valve9.onFor(SPICE_PUSH_HOLD_TIME); // Distributor #9 - pushing spice into packet
    Serial.println("SPICE PUSHED INTO PACKET");
}

void commandHolderExtend() {
    Serial.println("=== HOLDER EXTEND ===");
    Serial.print("Spice and packet holder cylinder on platform, extending holder for ");
    Serial.print(MS_TO_SECONDS(HOLD_SPICE_HOLD_TIME));
    Serial.println(" seconds");
    valve10.onFor(HOLD_SPICE_HOLD_TIME); // Distributor #10 - holding spice and packet on platform
    Serial.println("HOLDER EXTENDED");
}

void commandNozzleBack() {
    Serial.println("=== NOZZLE BACK ===");
    Serial.print("Moving nozzle back to packet start, vacuum position for ");
    Serial.print(MS_TO_SECONDS(NOZZLE_MOVE_HOLD_TIME));
    Serial.println(" seconds");
    valve11.offFor(NOZZLE_MOVE_HOLD_TIME); // Distributor #11 - nozzle movement for vacuum/sealing
    Serial.println("NOZZLE EXTENDED BACK");
}

void commandVacuumSeal() {
    Serial.println("=== VACUUM FOR SEALING ===");
    Serial.print("Switching valve to vacuum position, starting vacuum for ");
    Serial.print(MS_TO_SECONDS(VACUUM_HOLD_TIME));
    Serial.println(" seconds");
    
    // Переключення клапана на вакуум
    setVacuumValve(VALVE_POS_2); // Position 2 for vacuum
    Serial.println("VACUUM FOR SEALING COMPLETED");
}

void commandSealerDown() {
    Serial.println("=== SILICONE PLATE DOWN ===");
    Serial.print("Lowering silicone plate for packet sealing for ");
    Serial.print(MS_TO_SECONDS(SILICONE_BAR_HOLD_TIME));
    Serial.println(" seconds");
    valve12.onFor(SILICONE_BAR_HOLD_TIME); // Distributor #12 - silicone plate down/up movement
    Serial.println("SILICONE PLATE LOWERED");
}

void commandHeaterOn() {
    Serial.println("=== NOZZLE HEATING ===");
    Serial.print("Heating tape for sealing for ");
    Serial.print(MS_TO_SECONDS(HEATER_ACTIVE_TIME));
    Serial.println(" seconds");
    
    // Активний нагрів
    digitalWrite(heaterPin, HIGH); // Nozzle heater
    Serial.println("NOZZLE HEATING COMPLETED");
}

void commandVacuumOff() {
    Serial.println("=== VACUUM OFF ===");
    Serial.print("Turn off vacuum, switch valve to atmospheric position for ");
    Serial.print(MS_TO_SECONDS(VACUUM_RELEASE_TIME));
    Serial.println(" seconds");
    
    // Переключення клапана на атмосферу
    setVacuumValve(VALVE_POS_3); // Position 3 for atmosphere
    Serial.println("VACUUM DISABLED");
}

void commandSealerUp() {
    Serial.println("=== SILICONE PLATE UP ===");
    Serial.print("Raising silicone plate for packet sealing for ");
    Serial.print(MS_TO_SECONDS(SILICONE_BAR_HOLD_TIME));
    Serial.println(" seconds");
    valve12.offFor(SILICONE_BAR_HOLD_TIME); // Distributor #12 - plate raise
    Serial.println("SILICONE PLATE RAISED");
}

void commandCoolerOn() {
    Serial.println("=== COOLING ON ===");
    Serial.print("Enabling tape cooling for ");
    Serial.print(MS_TO_SECONDS(COOLER_ACTIVE_TIME));
    Serial.println(" seconds after sealing");
    valve14.onFor(COOLER_ACTIVE_TIME); // Distributor #14 - tape cooling after sealing
    Serial.println("COOLING COMPLETED");
}

void commandHolderUp() {
    Serial.println("=== HOLDER UP ===");
    Serial.print("Spice and packet holder cylinder on platform, raising for ");
    Serial.print(MS_TO_SECONDS(HOLD_SPICE_HOLD_TIME));
    Serial.println(" seconds, no holding");
    valve10.offFor(HOLD_SPICE_HOLD_TIME); // Distributor #10 - holder raise
    Serial.println("HOLDER RAISED");
}

void commandNozzleForward() {
    Serial.println("=== NOZZLE FORWARD ===");
    Serial.print("Moving nozzle forward, advancing packet forward for ");
    Serial.print(MS_TO_SECONDS(NOZZLE_MOVE_HOLD_TIME));
    Serial.println(" seconds");
    valve11.onFor(NOZZLE_MOVE_HOLD_TIME); // Distributor #11 - nozzle forward movement
    Serial.println("NOZZLE ADVANCED FORWARD");
}

void commandPusherHome() {
    Serial.println("=== PUSHER HOME ===");
    Serial.print("Returning cylinder that pushed spice to home position for ");
    Serial.print(MS_TO_SECONDS(SPICE_PUSH_HOLD_TIME));
    Serial.println(" seconds");
    valve9.offFor(SPICE_PUSH_HOLD_TIME); // Distributor #9 - pusher return
    Serial.println("PUSHER RETURNED TO HOME POSITION");
}

void commandPlatformReturnHome() {
    Serial.println("=== PLATFORM RETURN HOME ===");
    Serial.print("Returning suction platform to home position for ");
    Serial.print(MS_TO_SECONDS(PLATFORM_MOVE_HOLD_TIME));
    Serial.println(" seconds above packet storage");
    valve7.onFor(PLATFORM_MOVE_HOLD_TIME); // Distributor #7 - platform return
    Serial.println("PLATFORM RETURNED HOME");
}

void commandDropPacket() {
    Serial.println("=== DROP COMPLETED PACKET ===");
    Serial.print("Dropping completed packet from table for ");
    Serial.print(MS_TO_SECONDS(PACKAGE_DROP_HOLD_TIME));
    Serial.println(" seconds");
    valve13.onFor(PACKAGE_DROP_HOLD_TIME); // Distributor #13 - dropping completed packet
    Serial.println("COMPLETED PACKET DROPPED");
}

