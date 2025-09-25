#pragma once
#include <Arduino.h>
#include "pinout.h"
#include "config.h"

class Conveyor {
public:
    Conveyor() {}

    void begin() {
        pinMode(X_STEP_PIN, OUTPUT);
        pinMode(X_DIR_PIN, OUTPUT);
        pinMode(X_ENABLE_PIN, OUTPUT);
        // Y motor disabled: two motors wired to X driver
        // To restore Y as separate driver, re-enable the three lines below
        // pinMode(Y_STEP_PIN, OUTPUT);
        // pinMode(Y_DIR_PIN, OUTPUT);
        // pinMode(Y_ENABLE_PIN, OUTPUT);

        disable();
        // Y motor disabled: set only X direction. To restore Y, pass MOTOR_Y_DIR
        setDirection(MOTOR_X_DIR, /* MOTOR_Y_DIR */ MOTOR_X_DIR);

        running = false;
        dociagActive = false;
        dociagSteps = 0;
        dociagDone = 0;
        lastStepTime = 0;
        stepState = false;
    }

    void enable() {
        digitalWrite(X_ENABLE_PIN, LOW);
        // Y motor disabled: share X driver
        // digitalWrite(Y_ENABLE_PIN, LOW);
    }

    void disable() {
        digitalWrite(X_ENABLE_PIN, HIGH);
        // Y motor disabled: share X driver
        // digitalWrite(Y_ENABLE_PIN, HIGH);
    }

    void setDirection(bool xDir, bool yDir) {
        digitalWrite(X_DIR_PIN, xDir);
        // Y motor disabled: share X driver
        // digitalWrite(Y_DIR_PIN, yDir);
    }

    // Запустити постійний рух
    void start() {
        enable();
        running = true;
        dociagActive = false;
    }

    // Зупинити негайно
    void stop() {
        running = false;
        dociagActive = false;
        disable();
    }

    // Зупинка з дотягуванням (проїхати ще mm мм і зупинитись)
    void stopWithDociag(float mm) {
        if (mm <= 0) {
            stop();
            return;
        }
        // гарантуємо увімкнені драйвери для дотягування
        enable();
        dociagSteps = (unsigned long)(mm * STEPS_PER_MM_XY);
        dociagDone = 0;
        dociagActive = true;
    }

    // Основний update для генерації імпульсів
    void update() {
        unsigned long now = micros();

        // Якщо не рухаємося — нічого не робимо
        if (!running && !dociagActive) return;

        // Генеруємо імпульси з потрібною частотою
        if (!stepState && (now - lastStepTime >= STEP_INTERVAL_XY_MICROS)) {
            digitalWrite(X_STEP_PIN, HIGH);
            // Y motor disabled: share X driver
            // digitalWrite(Y_STEP_PIN, HIGH);
            stepState = true;
            lastStepTime = now;
        } else if (stepState && (now - lastStepTime >= PULSE_WIDTH_MICROS)) {
            digitalWrite(X_STEP_PIN, LOW);
            // Y motor disabled: share X driver
            // digitalWrite(Y_STEP_PIN, LOW);
            stepState = false;
            lastStepTime = now;

            // Якщо дотягування — рахуємо кроки
            if (dociagActive) {
                dociagDone++;
                if (dociagDone >= dociagSteps) {
                    dociagActive = false;
                    running = false;
                }
            }

        }
    }

    bool isRunning() const { return running || dociagActive; }
    bool isDociagActive() const { return dociagActive; }

private:
    bool running = false;
    bool dociagActive = false;
    unsigned long dociagSteps = 0;
    unsigned long dociagDone = 0;

    unsigned long lastStepTime = 0;
    bool stepState = false;
};

// Другий конвеєр (один двигун Z)
// Клас другого конвеєра (Z) видалено — перенесено на інший контролер