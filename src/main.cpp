#include <Arduino.h>
#include "pinout.h"
#include "config.h"
#include "pnevmatik_time.h"
#include "vacuumValve.h"
#include "pneumatic_valve.h"
#include "controls.h"
#include "conveyor.h"
#include "sensor_utils.h"

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

// Змінні для логіки роботи згідно алгоритму
uint8_t jarCounter = 0;           // Лічильник баночок (0-5)
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
const unsigned long DEBUG_INTERVAL = 4000; // 2 секунди між оновленнями відладки

void setup() {
  Serial.begin(9600);
  Serial.println("=== Machine Startup ===");
  
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
  controls.begin();
  conveyor.begin();
  
  // Sensor testing
  Serial.println("Testing sensors...");
  SensorUtils::testAllSensors(
    []() -> bool { return controls.isSensor1Active(); },
    []() -> bool { return controls.isSensor2Active(); },
    []() -> bool { return controls.isSensor3Active(); }
  );
  
  // Initial valve positions
  //valve1.on();
  //valve3.on();
  
  Serial.println("=== SETUP COMPLETED ===");
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

    // Отримуємо стан датчиків (через клас Controls)
    bool sensor1 = controls.isSensor1Active();
    bool sensor2 = controls.isSensor2Active();
    bool sensor3 = controls.isSensor3Active();

    // --- SENSOR LOGIC ACCORDING TO ALGORITHM ---
    
    // Sensor 1: Jar under paint dispensing nozzle
    if (sensor1 && !waitingForSensor1 && machineRunning && !machinePaused) {
        waitingForSensor1 = true;
        sensor1StartTime = millis();
        Serial.println("=== SENSOR 1: Jar under paint dispensing nozzle ===");
        
        // Stop conveyor with overrun for JAR_CENTERING_MM mm
        conveyor.stopWithDociag(JAR_CENTERING_MM);
        Serial.print("Conveyor stopping with overrun of ");
        Serial.print(JAR_CENTERING_MM);
        Serial.println(" mm for jar centering");
        
        // Paint cycle will start automatically after overrun completion
        // in main command execution cycle
    }
    
    // Sensor 2: Jar under cap closing press
    if (sensor2 && !waitingForSensor2 && machineRunning && !machinePaused) {
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
            // In single block mode - execute next command
            if (currentStep < machineProgramLength) {
                Serial.println("=== START (SINGLE STEP) ===");
                Serial.print("Next step: ");
                Serial.println(currentStep);
                
                // Execute current command
                machineProgram[currentStep]();
                
                // Move to next step
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
            // Normal mode - start machine
            machinePaused = false;
            conveyor.start();
            machineRunning = true;
            
            // Reset all counters and states
            jarCounter = 0;
            spiceSetCounter = 0;
            waitingForSensor1 = false;
            waitingForSensor2 = false;
            paintCycleActive = false;
            capCycleActive = false;
            packagingCycleActive = false;
            currentStep = 0;
            
            // Reset all timers
            sensor1StartTime = 0;
            sensor2StartTime = 0;
            paintCycleStartTime = 0;
            capCycleStartTime = 0;
            packagingCycleStartTime = 0;
            
            Serial.println("=== START PRESSED ===");
            Serial.println("Machine started - OPERATION mode");
            Serial.println("Conveyor enabled and started");
            Serial.println("All counters and states reset");
        }
    }
    
    // --- STOP Button ---
    if (stopBtn) {
        machinePaused = true;
        conveyor.stop();
        machineRunning = false;
        
        // Stop all active cycles
        paintCycleActive = false;
        capCycleActive = false;
        packagingCycleActive = false;
        
        // Reset all sensor waiting states
        waitingForSensor1 = false;
        waitingForSensor2 = false;
        
        // Reset all timers
        sensor1StartTime = 0;
        sensor2StartTime = 0;
        paintCycleStartTime = 0;
        capCycleStartTime = 0;
        packagingCycleStartTime = 0;
        
        Serial.println("=== STOP PRESSED ===");
        Serial.println("Machine stopped - PAUSE mode");
        Serial.println("Conveyor disabled and stopped");
        Serial.println("All active cycles stopped");
        Serial.println("All timers reset");
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
    
    // --- Mode Change Button ---
    if (modeBtn) {
        dispenseMode = !dispenseMode;
        Serial.println("=== DISPENSE MODE BUTTON PRESSED ===");
        if (dispenseMode) {
            Serial.println("Dispense mode: ALL JARS");
            Serial.println("Conveyor will stop for each jar");
            Serial.println("All 6 jars will be processed");
        } else {
            Serial.println("Dispense mode: ONE JAR");
            Serial.println("Conveyor will stop only for first jar");
            Serial.println("Only first jar of 6 will be processed");
        }
        
        // Reset jar counter when changing mode
        jarCounter = 0;
        Serial.println("Jar counter reset");
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
        Serial.println("/4");
        
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
        Serial.println("/4");
        
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
        conveyor.update();
        
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
            // Single block mode - execute one command at a time
            if (currentStep < machineProgramLength) {
                Serial.print("=== SINGLE STEP: Executing step ");
                Serial.print(currentStep);
                Serial.println(" ===");
                
                // Execute current command
                machineProgram[currentStep]();
                
                // Move to next command
                currentStep++;
                
                Serial.print("Step completed. Next step: ");
                Serial.println(currentStep);
                
                // If reached end - disable single block mode
                if (currentStep >= machineProgramLength) {
                    Serial.println("=== SINGLE STEP: All steps completed ===");
                    singleBlockMode = false;
                    currentStep = 0;
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
                Serial.print("=== PAINT CYCLE: Step ");
                Serial.print(currentStep);
                Serial.println(" ===");
                
                machineProgram[currentStep]();
                currentStep++;
                
                // Check if paint cycle completed
                if (currentStep > 4) {
                    paintCycleActive = false;
                    waitingForSensor1 = false;
                    jarCounter++;
                    
                    Serial.print("Paint cycle completed. Jar ");
                    Serial.print(jarCounter);
                    Serial.println(" processed");
                    
                    // Check dispense mode
                    if (dispenseMode || jarCounter == 0) {
                        // "All jars" mode or first jar - continue
                        if (jarCounter >= JARS_IN_SET - 1) {
                            // All jars processed
                            jarCounter = 0;
                            Serial.println("All jars in set processed");
                        }
                        conveyor.start();
                        Serial.println("Conveyor started for next jar");
                    } else {
                        // "One jar" mode - stop
                        Serial.println("'One jar' mode - waiting for next set");
                        machineRunning = false;
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
                Serial.print("=== CAP CLOSING CYCLE: Step ");
                Serial.print(currentStep);
                Serial.println(" ===");
                
                machineProgram[currentStep]();
                currentStep++;
                
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
                        
                        // If collected 4 spice sets - start packaging cycle
                        if (spiceSetCounter >= 4) {
                            packagingCycleActive = true;
                            packagingCycleStartTime = millis();
                            currentStep = 7; // Start with spice shift command
                            Serial.println("Packaging cycle started (4 spice sets ready)");
                        } else {
                            // Otherwise just shift spice set and continue
                            commandSpiceShift();
                            conveyor.start();
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
                Serial.print("=== PACKAGING CYCLE: Step ");
                Serial.print(currentStep);
                Serial.println(" ===");
                
                machineProgram[currentStep]();
                currentStep++;
                
                // Check if packaging cycle completed
                if (currentStep >= machineProgramLength) {
                    packagingCycleActive = false;
                    spiceSetCounter = 0;
                    
                    Serial.println("Packaging cycle completed");
                    conveyor.start();
                    Serial.println("Conveyor started for next set");
                    
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

void toggleDispenseMode() {
    // toggle dispense mode
}

void commandSpiceOut() {
    Serial.println("=== SPICE OUTPUT ===");
    bool sensor3 = controls.isSensor3Active();
    Serial.print("Sensor 3 (spice set ready): ");
    Serial.println(sensor3 ? "ACTIVE" : "INACTIVE");
    
    if (sensor3) {
        Serial.print("Activating spice output valve for ");
        Serial.print(MS_TO_SECONDS(SPICE_OUT_HOLD_TIME));
        Serial.println(" seconds");
        valve1.onFor(SPICE_OUT_HOLD_TIME); // Distributor #1 - spice output
        Serial.println("SPICE OUTPUT COMPLETED");
    } else {
        Serial.println("ERROR: Spice set not ready for output!");
    }
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

