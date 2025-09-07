#include <Arduino.h>
#include "pinout.h"
#include "config.h"
#include "pnevmatik_time.h"
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

// 4. Зсування спайки в положення для пакування
void commandSpiceShift();



bool machineRunning = false;
bool dispenseMode = false; // режим розливу (true - всі, false - лише перша)
bool machinePaused = false; // true — все стоїть, false — все працює
unsigned long pauseStartTime = 0; // момент старту паузи

// Ігнорування датчика 2 для наступних баночок після виконання циклу закривання
static uint8_t sensor2IgnoreCount = 0; // скільки наступних спрацювань S2 ігноруємо


// Змінні для логіки роботи згідно нового алгоритму
uint8_t jarCounter = 0;           // Лічильник баночок (0-5)
uint8_t jarsSeenInSet = 0;        // К-сть баночок з набору, що пройшли під датчиком 1 (0..6)
uint8_t spiceSetCounter = 0;      // Лічильник спайок (0-3, потрібно 4)
bool waitingForSensor1 = false;   // Очікування спрацювання датчика 1
bool waitingForSensor2 = false;   // Очікування спрацювання датчика 2
bool paintCycleActive = false;    // Активний цикл розливу фарби
bool capCycleActive = false;      // Активний цикл закривання кришок
bool spiceOutputDone = false;     // Чи виконано видачу спайок на початку циклу

// State variables for each cycle
uint8_t paintCycleStep = 0;       // Current step in paint cycle (0-3)
uint8_t capCycleStep = 0;         // Current step in cap cycle (0-5)
bool paintCycleDelayActive = false;
bool capCycleDelayActive = false;
unsigned long paintCycleDelayEnd = 0;
unsigned long capCycleDelayEnd = 0;

// Таймери для датчиків та циклів
unsigned long sensor1StartTime = 0;    // Час початку очікування датчика 1
unsigned long sensor2StartTime = 0;    // Час початку очікування датчика 2
unsigned long paintCycleStartTime = 0; // Час початку циклу розливу фарби
unsigned long capCycleStartTime = 0;   // Час початку циклу закривання кришок

// Змінні для відладки
unsigned long lastDebugTime = 0;
const unsigned long DEBUG_INTERVAL = 4000; // 4 секунди між оновленнями відладки

// --- Другий конвеєр: логіка чергування зсувів по датчику 3 ---
#if CONVEYOR_Z_SENSOR3_SHIFT_ENABLED
static bool conveyorZNextIsFirstOffset = true; // true: використовуємо перший зсув, false: другий
static unsigned long lastSensor3HandledMs = 0; // Антидребезг/мін. інтервал між спрацюваннями
static bool zDociagWasActivePrev = false;      // Для відстеження завершення дотягування
static bool zDwellActive = false;              // Активна затримка після зупинки
static unsigned long zDwellUntilMs = 0;        // Час завершення затримки
#endif

void setup() {
  Serial.begin(9600);
  Serial.println("=== Machine Startup ===");

  // Pneumatic valve initialization
  valve1.begin();
  valve2.begin();
  valve3.begin();
  valve4.begin();
  valve5.begin();
  valve6.begin();
  
  // Controls and conveyor initialization
  ControlsConfig cfg; // за замовчуванням: кнопки моментні, датчики без інверсії
  controls.begin(cfg);
  conveyor.begin();
  conveyorZ.begin();
  
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
    conveyor.update();
    conveyorZ.update();
    // Отримуємо стан датчиків (через клас Controls)
    bool sensor1 = controls.isSensor1Active();
   // bool sensor2 = controls.isSensor2Active();
    //bool sensor3 = controls.isSensor3Active();

    // --- SENSOR LOGIC ACCORDING TO ALGORITHM ---
    
    // Sensor 1: Jar under paint dispensing nozzle (edge-based, with mode logic)
    // Згідно з новим алгоритмом: видача фарби починається коли баночка доїхала до датчика 1
    static bool s1Prev = false;
    bool s1Rise = (!s1Prev && sensor1);
    s1Prev = sensor1;
    if (s1Rise && !waitingForSensor1 && machineRunning && !machinePaused && spiceOutputDone) {
    jarsSeenInSet++;
    if (!dispenseMode) {
        // Режим однієї баночки:
        // Розливаємо тільки першу баночку в наборі
        if (jarsSeenInSet == 1) {
            waitingForSensor1 = true;
            sensor1StartTime = millis();
            Serial.println("=== SENSOR 1: First jar in set - starting paint cycle ===");
            conveyor.stopWithDociag(JAR_CENTERING_MM);
        } else {
            Serial.print("S1: ONE-JAR mode, ignoring jar ");
            Serial.print(jarsSeenInSet);
            Serial.println(" in current set");
            // Продовжуємо рух конвеєра для наступних баночок
            conveyor.start();
        }
        
        // Скидаємо лічильник коли досягли кінця набору
        if (jarsSeenInSet >= JARS_IN_SET) {
            jarsSeenInSet = 0;
            Serial.println("ONE-JAR mode: Set completed, counter reset");
        }
    } else {
        // Режим всіх баночок - старий код
        waitingForSensor1 = true;
        sensor1StartTime = millis();
        Serial.println("=== SENSOR 1: Jar under paint dispensing nozzle ===");
        conveyor.stopWithDociag(JAR_CENTERING_MM);
    }
}

// Sensor 2: Jar under cap closing press (trigger on rising edge to avoid retrigger while jar stays under press)
    // Згідно з новим алгоритмом: цикл закривання кришок відбувається по спрацюванню датчика 2
    // Він знаходиться вкінці зони закривання, як тільки спрацював датчик він відразу зупиняє конвеєр 1
    bool s2Rise = controls.sensor2RisingEdge();
    if (s2Rise && !waitingForSensor2 && machineRunning && !machinePaused) {
        if (sensor2IgnoreCount > 0) {
            sensor2IgnoreCount--;
            Serial.print("S2: Ignored (remaining in set): ");
            Serial.println(sensor2IgnoreCount);
        } else {
        waitingForSensor2 = true;
        sensor2StartTime = millis();
        Serial.println("=== SENSOR 2: Jar under cap closing press ===");
        
        // Відразу зупиняє конвеєр 1
        conveyor.stop();
        Serial.println("Conveyor stopped immediately");
        
        // Відпрацьовується цикл закривання
        capCycleActive = true;
        capCycleStartTime = millis();
        capCycleStep = 0; // Start with cap screwing command
        Serial.println("Cap closing cycle started");
            // Після відпрацювання циклу закривання ігноруємо 5 наступних спрацювань датчика 2
            // так як баночок ми закриваєм по 6 шт зараз а спрацювання в нас відбувається по першій
            if (JARS_IN_SET > 1) {
                sensor2IgnoreCount = JARS_IN_SET - 1; // наприклад, 5
            }
        }
    }
    
    // Sensor 3: керування другим конвеєром по фронту (чергування зсувів)
    // Згідно з новим алгоритмом: зсування спайки в положення для запихання в пакет (розподілювач №6)
    // Даний розподілювач працює в парі з конвеєром Z та датчиком 3
    // Як тільки спрацював датчик 3 конвеєр Z спиняється із дотяжкою на потрібну довжину
#if CONVEYOR_Z_SENSOR3_SHIFT_ENABLED
    // Обробка фронту S3 лише коли машина у режимі RUN і не на паузі
    if (machineRunning && !machinePaused) {
        bool s3Rise = controls.sensor3RisingEdge();
        if (s3Rise) {
            unsigned long nowMs = millis();
            if (nowMs - lastSensor3HandledMs >= CONVEYOR_Z_MIN_TRIGGER_INTERVAL_MS) {
                lastSensor3HandledMs = nowMs;
                // Є дві величини дотягування в config.h це параметри CONVEYOR_Z_OFFSET_MM_FIRST та CONVEYOR_Z_OFFSET_MM_SECOND
                // для компактнішого розположення було придумано складати їх в шахмотному порядку, тому і дві величини зсуву
                // для 1 і 3 застосовуєм перший параметр а для 2 і 4 другий
                float shiftMm = conveyorZNextIsFirstOffset ? CONVEYOR_Z_OFFSET_MM_FIRST : CONVEYOR_Z_OFFSET_MM_SECOND;
                conveyorZNextIsFirstOffset = !conveyorZNextIsFirstOffset; // чергуємо 1-2-1-2
                Serial.print("S3 RISE: Conveyor Z stopWithDociag ");
                Serial.print(shiftMm);
                Serial.println(" mm (alternating)");
                conveyorZ.stopWithDociag(shiftMm);
                // Після запуску дотягування — скинути dwell, він активується після завершення
                zDwellActive = false;

                // Згідно з новим алгоритмом: зсування спайки в положення для запихання в пакет (розподілювач №6)
                // Як тільки закінчилась дотяжка конвеєр Z стоїть поки не завершить роботу пнематика
                // тому включаєм на заданий час розподілювач №6 і по заершеню часу він вертається в початкове положення
                Serial.println("Executing spice shift command (Distributor #6)");
                commandSpiceShift();

                // Запуск етапів пакування: пункт 4 (SPICE SHIFT)
                if (spiceSetCounter >= SPICE_SETS_PER_PACKAGE) {
                    spiceSetCounter = 0;
                    Serial.println("Spice set counter reset after reaching required sets");
                }
            }
        }
    }
    // Відстеження завершення дотягування й запуск затримки (dwell)
    if (conveyorZ.isDociagActive()) {
        zDociagWasActivePrev = true;
    } else if (zDociagWasActivePrev) {
        zDociagWasActivePrev = false;
        if (CONVEYOR_Z_DWELL_MS > 0) {
            zDwellActive = true;
            zDwellUntilMs = millis() + CONVEYOR_Z_DWELL_MS;
            Serial.print("Conveyor Z dwell started for ");
            Serial.print(CONVEYOR_Z_DWELL_MS);
            Serial.println(" ms");
        }
    }
    // Якщо dwell активний — чекаємо
    if (zDwellActive && (long)(millis() - zDwellUntilMs) >= 0) {
        zDwellActive = false;
        Serial.println("Conveyor Z dwell finished");
    }
    // Автостарт Z після дотягування і затримки лише коли RUN і не PAUSE
    if (machineRunning && !machinePaused && !conveyorZ.isRunning() && !zDwellActive) {
        conveyorZ.start();
    }
#endif

    // Отримуємо стан кнопок
    bool startBtn = controls.startPressed();
    bool stopBtn = controls.stopPressed();
    bool modeBtn = controls.modeChanged();

        // --- START Button ---
    if (startBtn) {
        {
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

                // Зсунути алгоритмічні таймери, якщо вони були активні (ненульові)
                if (sensor1StartTime)       sensor1StartTime       += delta;
                if (sensor2StartTime)       sensor2StartTime       += delta;
                if (paintCycleStartTime)    paintCycleStartTime    += delta;
                if (capCycleStartTime)      capCycleStartTime      += delta;
                if (paintCycleDelayActive)  paintCycleDelayEnd     += delta;
                if (capCycleDelayActive)    capCycleDelayEnd       += delta;

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
                paintCycleStep = 0;
                capCycleStep = 0;
                spiceOutputDone = false;

                // Скидання таймерів
                sensor1StartTime = 0;
                sensor2StartTime = 0;
                paintCycleStartTime = 0;
                capCycleStartTime = 0;

                Serial.println("=== START PRESSED ===");
                Serial.println("Machine started - OPERATION mode");
                Serial.println("Conveyors enabled and started");
                Serial.println("Counters and states initialized");

                // Згідно з новим алгоритмом: ПЕРШИЙ крок - видача спайок (розподілювач №1)
                // Циліндр видвигається на 1с і вертається назад, до того як він почав рух запустився основний конвеєр
                Serial.println("Executing initial step: SPICE OUTPUT (Distributor #1)");
                commandSpiceOut();
                spiceOutputDone = true;
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
    
    // --- Mechanism updates only if not paused ---
    if (!machinePaused) {
        // Update conveyor always (it controls its own state)
        //conveyor.update();
        
        // Additional debugging for understanding state
        static unsigned long lastStatusCheck = 0;
        if (millis() - lastStatusCheck > 1000) { // Every second
            lastStatusCheck = millis();
            
            if (machineRunning) {
                if (paintCycleActive) {
                    Serial.print("PAINT CYCLE: Step ");
                    Serial.println(paintCycleStep);
                } else if (capCycleActive) {
                    Serial.print("CAP CLOSING CYCLE: Step ");
                    Serial.println(capCycleStep);
                } else {
                    Serial.println("AUTO MODE: Waiting for sensor triggers");
                }
            } else {
                Serial.println("Machine idle - press START");
            }
        }
    }

    // --- COMMAND EXECUTION LOGIC ACCORDING TO ALGORITHM ---
    if (!machinePaused) {
        if (machineRunning) {
            // Automatic mode - execute commands according to active cycles
            

            // Check if overrun completed for sensor 1
            if (waitingForSensor1 && !conveyor.isRunning() && !paintCycleActive) {
                // Overrun completed, start paint cycle
                paintCycleActive = true;
                paintCycleStartTime = millis();
                paintCycleStep = 0;
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
            
            // Paint dispensing cycle (steps 0-3)
            if (paintCycleActive) {
                if (!paintCycleDelayActive) {
                    Serial.print("=== PAINT CYCLE: Step ");
                    Serial.print(paintCycleStep);
                    Serial.println(" ===");
                    
                    // Execute current step
                    switch (paintCycleStep) {
                        case 0:
                            commandPaintValveOpen();
                            break;
                        case 1:
                            commandPaintPistonIntake();
                            break;
                        case 2:
                            commandPaintValveClose();
                            break;
                        case 3:
                            commandPaintPistonDispense();
                            break;
                    }
                    
                    // Set delay for current step
                    unsigned long delayMs = 0;
                    switch (paintCycleStep) {
                        case 0: delayMs = STEP_PAUSE_PAINT_VALVE_OPEN_MS; break;
                        case 1: delayMs = STEP_PAUSE_PAINT_PISTON_INTAKE_MS; break;
                        case 2: delayMs = STEP_PAUSE_PAINT_VALVE_CLOSE_MS; break;
                        case 3: delayMs = STEP_PAUSE_PAINT_PISTON_DISPENSE_MS; break;
                    }
                    
                    if (delayMs > 0) {
                        paintCycleDelayActive = true;
                        paintCycleDelayEnd = millis() + delayMs;
                    }
                    
                    paintCycleStep++;
                } else if (millis() >= paintCycleDelayEnd) {
                    paintCycleDelayActive = false;
                }
                
                // Check if paint cycle completed
                if (paintCycleStep > 3) {
                    paintCycleActive = false;
                    waitingForSensor1 = false;
                    jarCounter++;
                    Serial.print("Paint cycle completed. Jar ");
                    Serial.print(jarCounter);
                    Serial.println(" processed");

                    // Згідно з новим алгоритмом: після завершення розливу фарби конвеєр продовжує рух
                    if (dispenseMode) {
                        // Розливати усі 6 баночок
                        if (jarCounter >= JARS_IN_SET) {
                            jarCounter = 0;
                            jarsSeenInSet = 0;
                            Serial.println("All jars in set processed");
                        }
                        conveyor.start();
                        Serial.println("Conveyor started for next jar");
                    } else {
                        // Розливати лишень першу баночку і тоді 5 наступних ігнорувати
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
            
            // Cap closing cycle (steps 0-4)
            else if (capCycleActive) {
                if (!capCycleDelayActive) {
                    Serial.print("=== CAP CLOSING CYCLE: Step ");
                    Serial.print(capCycleStep);
                    Serial.println(" ===");
                    
                    // Execute current step
                    switch (capCycleStep) {
                        case 0:
                            // Клапан 4 (завертання) ВКЛЮЧАЄТЬСЯ і утримується
                            Serial.println("Step 0: Turning ON valve 4 (cap screwing)");
                            valve4.on();
                            break;
                        case 1:
                            // Затримка перед включенням клапана 5
                            Serial.println("Step 1: Delay before turning ON valve 5");
                            break;
                        case 2:
                            // Клапан 5 (закривання) ВКЛЮЧАЄТЬСЯ на 2000мс
                            Serial.println("Step 2: Turning ON valve 5 (cap closing) for 2000ms");
                            valve5.on();
                            break;
                        case 3:
                            // Клапан 5 (закривання) ВИМИКАЄТЬСЯ
                            Serial.println("Step 3: Turning OFF valve 5 (cap closing)");
                            if (valve5.isOn()) valve5.off();
                            break;
                        case 4:
                            // Затримка перед вимкненням клапана 4
                            Serial.println("Step 4: Delay before turning OFF valve 4");
                            break;
                        case 5:
                            // Клапан 4 (завертання) ВИМИКАЄТЬСЯ
                            Serial.println("Step 5: Turning OFF valve 4 (cap screwing)");
                            if (valve4.isOn()) valve4.off();
                            break;
                    }
                    
                    // Set delay for current step
                    unsigned long delayMs = 0;
                    switch (capCycleStep) {
                        case 0: delayMs = 10; break; // Час утримання клапана 4
                        case 1: delayMs = STEP_PAUSE_CAP_SCREW_MS; break; // Затримка перед клапаном 5
                        case 2: delayMs = CLOSE_CAP_HOLD_TIME; break; // Час роботи клапана 5
                        case 3: delayMs = 10; break; // Мінімальна затримка після вимкнення клапана 5
                        case 4: delayMs = STEP_PAUSE_CAP_CLOSE_MS; break; // Затримка перед вимкненням клапана 4
                        case 5: delayMs = 10; break; // Мінімальна затримка після вимкнення клапана 4
                    }
                    
                    if (delayMs > 0) {
                        capCycleDelayActive = true;
                        capCycleDelayEnd = millis() + delayMs;
                    }
                    
                    capCycleStep++;
                } else if (millis() >= capCycleDelayEnd) {
                    capCycleDelayActive = false;
                }
                
                // Check if cap closing cycle completed
                if (capCycleStep > 5) {
                    capCycleActive = false;
                    waitingForSensor2 = false;
                    
                    Serial.println("Cap closing cycle completed");
                    // Згідно з новим алгоритмом: після відпрацювання циклу закривання відновлюється рух конвеєра
                    conveyor.start();
                    Serial.println("Conveyor restarted after cap closing cycle");
                }
                
                // Check for cap cycle timeout
                if (capCycleActive && IS_TIMEOUT(capCycleStartTime, CAP_CYCLE_TOTAL_TIME)) {
                    Serial.println("WARNING: Cap closing cycle timeout - restarting conveyor");
                    capCycleActive = false;
                    waitingForSensor2 = false;
                    conveyor.start();
                }
            }
            
            
        }
    }
}

void commandSpiceOut() {
    Serial.println("=== SPICE OUTPUT (Distributor #1) ===");
    valve1.onFor(SPICE_OUT_HOLD_TIME); // Distributor #1 - spice output
    Serial.println("SPICE OUTPUT COMPLETED");
}

void commandPaintValveOpen() {
    Serial.println("=== PAINT VALVE OPEN (Distributor #2) ===");
    bool sensor1 = controls.isSensor1Active();
    
    if (sensor1) {
        valve2.on(); // Distributor #2 - reservoir valve open
        Serial.println("PAINT VALVE OPENED");
    } else {
        Serial.println("ERROR: No jar under nozzle!");
    }
}

void commandPaintPistonIntake() {
    Serial.println("=== PAINT PISTON INTAKE (Distributor #3) ===");
    bool sensor1 = controls.isSensor1Active();
    
    if (sensor1) {
        valve3.on(); // Distributor #3 - piston movement for paint intake
        Serial.println("PAINT PISTON INTAKE COMPLETED");
    } else {
        Serial.println("ERROR: No jar under nozzle!");
    }
}

void commandPaintValveClose() {
    Serial.println("=== PAINT VALVE CLOSE (Distributor #2) ===");
    valve2.off(); // Distributor #2 - reservoir valve close
    Serial.println("PAINT VALVE CLOSED");
}

void commandPaintPistonDispense() {
    Serial.println("=== PAINT PISTON DISPENSE (Distributor #3) ===");
    bool sensor1 = controls.isSensor1Active();
    
    if (sensor1) {
        valve3.off(); // Distributor #3 - piston movement for paint dispensing
        Serial.println("PAINT PISTON DISPENSE COMPLETED");
    } else {
        Serial.println("ERROR: No jar under nozzle!");
    }
}


void commandSpiceShift() {
    Serial.println("=== SPICE SHIFT (Distributor #6) ===");
    valve6.onFor(SPICE_SHIFT_HOLD_TIME); // Distributor #6 - spice shift for packaging
    Serial.println("SPICE SHIFT COMPLETED");
}
