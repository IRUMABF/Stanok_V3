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
        pinMode(Y_STEP_PIN, OUTPUT);
        pinMode(Y_DIR_PIN, OUTPUT);
        pinMode(Y_ENABLE_PIN, OUTPUT);

        disable();
        setDirection(MOTOR_X_DIR, MOTOR_Y_DIR);

        running = false;
        dociagActive = false;
        dociagSteps = 0;
        dociagDone = 0;
        lastStepTime = 0;
        stepState = false;
    }

    void enable() {
        digitalWrite(X_ENABLE_PIN, LOW);
        digitalWrite(Y_ENABLE_PIN, LOW);
    }

    void disable() {
        digitalWrite(X_ENABLE_PIN, HIGH);
        digitalWrite(Y_ENABLE_PIN, HIGH);
    }

    void setDirection(bool xDir, bool yDir) {
        digitalWrite(X_DIR_PIN, xDir);
        digitalWrite(Y_DIR_PIN, yDir);
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
        dociagSteps = (unsigned long)(mm * STEPS_PER_MM);
        dociagDone = 0;
        dociagActive = true;
    }

    // Основний update для генерації імпульсів
    void update() {
        unsigned long now = micros();

        // Якщо не рухаємося — нічого не робимо
        if (!running && !dociagActive) return;

        // Генеруємо імпульси з потрібною частотою
        if (!stepState && (now - lastStepTime >= STEP_INTERVAL_MICROS)) {
            digitalWrite(X_STEP_PIN, HIGH);
            digitalWrite(Y_STEP_PIN, HIGH);
            stepState = true;
            lastStepTime = now;
        } else if (stepState && (now - lastStepTime >= PULSE_WIDTH_MICROS)) {
            digitalWrite(X_STEP_PIN, LOW);
            digitalWrite(Y_STEP_PIN, LOW);
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
class ConveyorZ {
public:
    ConveyorZ() {}

    void begin() {
        pinMode(Z_STEP_PIN, OUTPUT);
        pinMode(Z_DIR_PIN, OUTPUT);
        pinMode(Z_ENABLE_PIN, OUTPUT);

        disable();
        setDirection(MOTOR_Z_DIR);

        running = false;
        dociagActive = false;
        dociagSteps = 0;
        dociagDone = 0;
        lastStepTime = 0;
        stepState = false;
    }

    void enable() { digitalWrite(Z_ENABLE_PIN, LOW); }
    void disable() { digitalWrite(Z_ENABLE_PIN, HIGH); }

    void setDirection(bool zDir) { digitalWrite(Z_DIR_PIN, zDir); }

    void start() {
        enable();
        running = true;
        dociagActive = false;
    }

    void stop() {
        running = false;
        dociagActive = false;
        disable();
    }

    void stopWithDociag(float mm) {
        if (mm <= 0) { stop(); return; }
        enable();
        dociagSteps = (unsigned long)(mm * STEPS_PER_MM);
        dociagDone = 0;
        dociagActive = true;
    }

    void update() {
        unsigned long now = micros();
        if (!running && !dociagActive) return;

        if (!stepState && (now - lastStepTime >= STEP_INTERVAL_MICROS)) {
            digitalWrite(Z_STEP_PIN, HIGH);
            stepState = true;
            lastStepTime = now;
        } else if (stepState && (now - lastStepTime >= PULSE_WIDTH_MICROS)) {
            digitalWrite(Z_STEP_PIN, LOW);
            stepState = false;
            lastStepTime = now;

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