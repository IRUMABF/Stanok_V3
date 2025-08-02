#include <Arduino.h>
#include <TimerOne.h> // Додаємо бібліотеку
#include "config.h"
#include "pins.h"

volatile bool stepState = false;
bool conveyorRunning = true;
bool waitForJarToLeave = false;
bool waitForCapToLeave = false;

// Add control panel variables
bool machineRunning = true;  // Machine state (running/stopped)
bool dispenseMode = false;   // false = mode 0 (all jars), true = mode 1 (first jar only)
bool prevStartButtonState = HIGH;
bool prevModeButtonState = HIGH;

int jarCounter = 0;
int capCounter = 0;

// Додаємо змінні для debounce
unsigned long lastStartButtonChange = 0;
unsigned long lastModeButtonChange = 0;
const unsigned long debounceDelay = 50; // 50 мс

// ISR для кроку двигуна
void stepConveyorISR() {
  if (!conveyorRunning) return;
  digitalWrite(X_STEP_PIN, HIGH);
  digitalWrite(Y_STEP_PIN, HIGH);
  delayMicroseconds(PULSE_WIDTH_MICROS);
  digitalWrite(X_STEP_PIN, LOW);
  digitalWrite(Y_STEP_PIN, LOW);
}

void moveConveyor() {
  // Порожня, логіку кроку тепер виконує таймер
}

void printSensorStates() {
  bool yMinState = digitalRead(Y_MIN_PIN) == LOW;
  bool yMaxState = digitalRead(Y_MAX_PIN) == LOW;
  Serial.print("Y_MIN (під соплом): ");
  Serial.print(yMinState ? "Об'єкт присутній" : "Немає об'єкта");
  Serial.print(" | Y_MAX (під пресом): ");
  Serial.println(yMaxState ? "Об'єкт присутній" : "Немає об'єкта");
}

void extrudePaint() {
  Serial.println("Екструзія фарби...");
  delay(1000); // імітація екструзії фарби (1 сек)
  Serial.println("Екструзія завершена.");
}

void moveConveyorDistance(float mm) {
  unsigned long steps = (unsigned long)(mm * STEPS_PER_MM);
  Serial.print("Додатковий рух конвеєра на ");
  Serial.print(mm);
  Serial.println(" мм для центрування баночки.");
  for (unsigned long i = 0; i < steps; i++) {
    digitalWrite(X_STEP_PIN, HIGH);
    digitalWrite(Y_STEP_PIN, HIGH);
    delayMicroseconds(PULSE_WIDTH_MICROS);
    digitalWrite(X_STEP_PIN, LOW);
    digitalWrite(Y_STEP_PIN, LOW);
    delayMicroseconds(STEP_INTERVAL_MICROS - PULSE_WIDTH_MICROS);
  }
}

void closeCapPneumatic() {
  Serial.println("Закривання кришки пневматикою (2 циліндри)...");

  // 1. Завертання кришки (PNEUMATIC_01_PIN) - вперед
  digitalWrite(PNEUMATIC_00_PIN, HIGH);
  delay(TWIST_FORWARD_TIME);

  // 2. Закривання (PNEUMATIC_02_PIN) - вперед
  digitalWrite(PNEUMATIC_01_PIN, HIGH);
  delay(CAP_FORWARD_TIME);

  // 3. Закривання (PNEUMATIC_02_PIN) - назад
  digitalWrite(PNEUMATIC_01_PIN, LOW);
  delay(CAP_BACKWARD_TIME);

  // 4. Завертання кришки (PNEUMATIC_01_PIN) - назад
  digitalWrite(PNEUMATIC_00_PIN, LOW);
  delay(TWIST_BACKWARD_TIME);

  Serial.println("Кришка закрита (2 циліндри).");
}

void setup() {
  Serial.begin(115200);

  pinMode(Y_MIN_PIN, INPUT_PULLUP);
  pinMode(Y_MAX_PIN, INPUT_PULLUP);

  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);
  pinMode(X_ENABLE_PIN, OUTPUT);

  pinMode(Y_STEP_PIN, OUTPUT);
  pinMode(Y_DIR_PIN, OUTPUT);
  pinMode(Y_ENABLE_PIN, OUTPUT);

  pinMode(PNEUMATIC_00_PIN, OUTPUT);
  pinMode(PNEUMATIC_01_PIN, OUTPUT);

  // Initialize control panel pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(modeButtonPin, INPUT_PULLUP);
 // pinMode(GND_BUTTON_PIN, OUTPUT);
  //pinMode(ledGndPin, OUTPUT);
  pinMode(ledMode0Pin, OUTPUT);
  pinMode(ledMode1Pin, OUTPUT);
  
  // Set GND pins
  //digitalWrite(GND_BUTTON_PIN, LOW);
  //digitalWrite(ledGndPin, LOW);
  
  // Initial LED states
  digitalWrite(ledMode0Pin, HIGH);  // Mode 0 active by default
  digitalWrite(ledMode1Pin, LOW);

  digitalWrite(X_ENABLE_PIN, LOW);
  digitalWrite(Y_ENABLE_PIN, LOW);
  digitalWrite(PNEUMATIC_00_PIN, LOW);
  digitalWrite(PNEUMATIC_01_PIN, LOW);


  digitalWrite(X_DIR_PIN, MOTOR_X_DIR);
  digitalWrite(Y_DIR_PIN, MOTOR_Y_DIR);

  // Ініціалізуємо таймер для кроків
  Timer1.initialize(STEP_INTERVAL_MICROS); // Інтервал між кроками
  Timer1.attachInterrupt(stepConveyorISR);
}

void loop() {
  static bool lastMachineRunning = machineRunning;

  bool currentStartButton = digitalRead(BUTTON_PIN);
  bool currentModeButton = digitalRead(modeButtonPin);
  unsigned long now = millis();

  // Debounce для тумблера старт/стоп
  if (currentStartButton != prevStartButtonState && (now - lastStartButtonChange) > debounceDelay) {
    lastStartButtonChange = now;
    prevStartButtonState = currentStartButton;
    machineRunning = (currentStartButton == HIGH); // або LOW, залежно від підключення
    conveyorRunning = machineRunning;
    Serial.println(machineRunning ? "Machine started" : "Machine stopped");
  }

  // Debounce для тумблера режиму (тільки коли машина зупинена)
  if (!machineRunning && currentModeButton != prevModeButtonState && (now - lastModeButtonChange) > debounceDelay) {
    lastModeButtonChange = now;
    prevModeButtonState = currentModeButton;
    dispenseMode = (currentModeButton == HIGH); // або LOW, залежно від підключення
    digitalWrite(ledMode0Pin, !dispenseMode);
    digitalWrite(ledMode1Pin, dispenseMode);
    Serial.println(dispenseMode ? "Mode 1: First jar only" : "Mode 0: All jars");
  }

  // --- ОНОВЛЕННЯ РЕЖИМУ ПРИ ЗУПИНЦІ ---
  if (lastMachineRunning && !machineRunning) {
    // Машина тільки-но зупинилася, одразу оновлюємо режим за станом тумблера
    bool actualModeButton = digitalRead(modeButtonPin);
    dispenseMode = (actualModeButton == HIGH); // або LOW, залежно від підключення
    digitalWrite(ledMode0Pin, !dispenseMode);
    digitalWrite(ledMode1Pin, dispenseMode);
    Serial.print("Режим оновлено при зупинці: ");
    Serial.println(dispenseMode ? "Mode 1: First jar only" : "Mode 0: All jars");
    prevModeButtonState = actualModeButton; // синхронізуємо стан
  }
  lastMachineRunning = machineRunning;

  // Only process machine logic if running
  if (machineRunning) {
    static unsigned long lastSensorPoll = 0;
    unsigned long nowMillis = millis();
    if (nowMillis - lastSensorPoll >= SENSOR_POLL_INTERVAL) {
      lastSensorPoll = nowMillis;
      printSensorStates();

      bool yMinState = digitalRead(Y_MIN_PIN) == LOW;

      if (!waitForJarToLeave && yMinState) {
        bool allowDispense = true;
        if (dispenseMode) {
          // Mode 1: Dispense only first jar
          if (jarCounter >= 1) allowDispense = false;
        }
        if (allowDispense) {
          conveyorRunning = false;
          Serial.println("Баночка під соплом. Зупинка конвеєра.");
          moveConveyorDistance(JAR_CENTERING_MM);
          extrudePaint();
          conveyorRunning = true;
          Serial.println("Екструзія завершена. Запуск конвеєра для виїзду баночки.");
          waitForJarToLeave = true;
          jarCounter++;
        } else {
          // Просто чекаємо, поки баночка виїде, без розливу
          waitForJarToLeave = true;
          Serial.println("Баночка під соплом, але розлив ігнорується (режим 1).");
          jarCounter++;
        }
        if (jarCounter >= JARS_IN_SET) jarCounter = 0; // Скидаємо лічильник після 6 баночок
      }

      if (waitForJarToLeave && !yMinState) {
        waitForJarToLeave = false;
        Serial.println("Баночка виїхала. Готово до наступної.");
      }

      bool yMaxState = digitalRead(Y_MAX_PIN) == LOW;

      if (!waitForCapToLeave && yMaxState) {
        if (capCounter == 0) { // Закривання лише для першої баночки у збірці
          conveyorRunning = false;
          Serial.println("Баночка під пресом. Зупинка конвеєра.");
          closeCapPneumatic();
          conveyorRunning = true;
          Serial.println("Кришка закрита. Запуск конвеєра для виїзду баночки.");
        } else {
          Serial.println("Баночка під пресом, але закривання ігнорується (збірка).");
        }
        waitForCapToLeave = true;
        capCounter++;
        if (capCounter >= JARS_IN_SET) capCounter = 0;
      }

      if (waitForCapToLeave && !yMaxState) {
        waitForCapToLeave = false;
        Serial.println("Баночка виїхала з-під преса. Готово до наступної.");
      }
    }

    // тут можна додати інший код
  }
}
