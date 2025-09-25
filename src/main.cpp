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
PneumaticValve valve3(PNEUMATIC_3_PIN);
PneumaticValve valve4(PNEUMATIC_4_PIN);
PneumaticValve valve5(PNEUMATIC_5_PIN); 
Controls controls;
Conveyor conveyor;      // Основний конвеєр (X+Y)

// --- Прототипи команд ---
// 1. Видача спайок
void commandSpiceOut();

// 2. Розлив фарби: один імпульс поршня на заданий час
void commandPaintPulse();

// --- Індикація режимів ---
extern bool dispenseMode; // forward declaration
static inline void updateModeLeds() {
  // true => ALL JARS (режим 0), false => ONE JAR (режим 1)
  digitalWrite(ledMode0Pin, dispenseMode ? HIGH : LOW);
  digitalWrite(ledMode1Pin, dispenseMode ? LOW : HIGH);
}

// 4. Зсування спайки перенесено на інший контролер



bool machineRunning = false;
bool dispenseMode = false; // режим розливу (true - всі, false - лише перша)
bool machinePaused = false; // true — все стоїть, false — все працює
unsigned long pauseStartTime = 0; // момент старту паузи

// Ігнорування датчика 2 для наступних баночок після виконання циклу закривання
static uint8_t sensor2IgnoreCount = 0; // скільки наступних спрацювань S2 ігноруємо


// Змінні для логіки роботи згідно нового алгоритму
uint8_t jarCounter = 0;           // Лічильник баночок (0-5)
uint8_t jarsSeenInSet = 0;        // К-сть баночок з набору, що пройшли під датчиком 1 (0..6)
// uint8_t spiceSetCounter = 0;      // перенесено
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

 

void setup() {
  Serial.begin(9600);
  Serial.println("=== Machine Startup ===");
  // Pneumatic valve initialization
  valve1.begin();
  valve3.begin();
  valve4.begin();
  valve5.begin();
  
  
  // Controls and conveyor initialization
  ControlsConfig cfg; // за замовчуванням: кнопки моментні, датчики без інверсії
  controls.begin(cfg);
  conveyor.begin();
  // LED pins for mode indication
  pinMode(ledMode0Pin, OUTPUT);
  pinMode(ledMode1Pin, OUTPUT);
  updateModeLeds();
  
  // Initialize START_STOP_PIN output
  pinMode(START_STOP_PIN, OUTPUT);
  digitalWrite(START_STOP_PIN, LOW); // Initially machine is stopped
  
  Serial.println("System ready for operation!");
  Serial.println("=====================================");
}

void loop() {
    // Оновлення стану кнопок і датчиків
    controls.update();
    
    // Оновлення таймерів клапанів (ОБОВ'ЯЗКОВО!)
    valve1.update();
    valve3.update();
    valve4.update();
    valve5.update();
    
    conveyor.update();
    // Отримуємо стан датчиків (через клас Controls)
    bool sensor1 = controls.isSensor1Active();
   // bool sensor2 = controls.isSensor2Active();
    

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

                // Зсунути таймери клапанів
                valve1.shiftTimers(delta);  valve3.shiftTimers(delta);
                valve4.shiftTimers(delta);  valve5.shiftTimers(delta);

                // Зсунути алгоритмічні таймери, якщо вони були активні (ненульові)
                if (sensor1StartTime)       sensor1StartTime       += delta;
                if (sensor2StartTime)       sensor2StartTime       += delta;
                if (paintCycleStartTime)    paintCycleStartTime    += delta;
                if (capCycleStartTime)      capCycleStartTime      += delta;
                if (paintCycleDelayActive)  paintCycleDelayEnd     += delta;
                if (capCycleDelayActive)    capCycleDelayEnd       += delta;

                Serial.println("=== RESUME ===");
                Serial.println("Resuming machine without state reset");
                digitalWrite(START_STOP_PIN, HIGH); // Set high on resume
            } else {
                // Перший старт — ініціалізація циклу
                machinePaused = false;
                conveyor.start();
                machineRunning = true;

                // Скидання лічильників та станів лише при першому старті
                jarCounter = 0;
                // spiceSetCounter = 0;
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
                digitalWrite(START_STOP_PIN, HIGH); // Set high on start

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
        if (!machinePaused) {
            machinePaused = true;
            pauseStartTime = millis();
            conveyor.stop();
            machineRunning = false;
            digitalWrite(START_STOP_PIN, LOW); // Set low on stop
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
            updateModeLeds();
        }
    } else if (modeBtn) {
        // Momentary button: toggle mode on press
        dispenseMode = !dispenseMode;
        Serial.println("=== DISPENSE MODE BUTTON PRESSED ===");
        Serial.println(dispenseMode ? "Mode: ALL JARS" : "Mode: ONE JAR");
        jarCounter = 0;
        jarsSeenInSet = 0;
        updateModeLeds();
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
            
            // Paint dispensing cycle (single step)
            if (paintCycleActive) {
                if (!paintCycleDelayActive) {
                    Serial.print("=== PAINT CYCLE: Step ");
                    Serial.print(paintCycleStep);
                    Serial.println(" ===");
                    
                    // Execute current step
                    commandPaintPulse();
                    
                    // No intra-step delay; command uses its own hold time
                    unsigned long delayMs = 0;
                    
                    if (delayMs > 0) {
                        paintCycleDelayActive = true;
                        paintCycleDelayEnd = millis() + delayMs;
                    }
                    
                    paintCycleStep++;
                } else if (millis() >= paintCycleDelayEnd) {
                    paintCycleDelayActive = false;
                }
                
                // Check if paint cycle completed
                if (paintCycleStep > 0) {
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

void commandPaintPulse() {
    Serial.println("=== PAINT PISTON PULSE (Distributor #3) ===");
    bool sensor1 = controls.isSensor1Active();
    if (sensor1) {
        valve3.onFor(PAINT_PISTON_HOLD_TIME);
        Serial.println("PAINT PULSE COMMANDED");
    } else {
        Serial.println("ERROR: No jar under nozzle!");
    }
}