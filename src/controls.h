#pragma once
#include <Arduino.h>
#include "pinout.h"

struct ButtonState {
    bool current = false;
    bool last = false;
    unsigned long lastChange = 0;
    bool pressedEvent = false;
};

class Controls {
public:
    // Ініціалізація всіх пінів: кнопок та датчиків
    void begin() {
        // Кнопки
        pinMode(start_PIN, INPUT_PULLUP);
        pinMode(stop_PIN, INPUT_PULLUP);
        pinMode(modeButtonPin, INPUT_PULLUP);
        pinMode(singleblockButtonPin, INPUT_PULLUP);

        // Датчики (INPUT_PULLUP - активний стан = LOW)
        pinMode(sensor_1, INPUT_PULLUP);
        pinMode(sensor_2, INPUT_PULLUP);
        pinMode(sensor_3, INPUT_PULLUP);
    }

    // Оновлення стану всіх кнопок та датчиків
    void update() {
        // Оновлення кнопок
        updateButton(start_PIN, startBtn);
        updateButton(stop_PIN, stopBtn);
        updateButton(modeButtonPin, modeBtn);
        updateButton(singleblockButtonPin, singleBlockBtn);

        // Оновлення датчиків (INPUT_PULLUP: активний = LOW)
        sensor1State = (digitalRead(sensor_1) == LOW);
        sensor2State = (digitalRead(sensor_2) == LOW);
        sensor3State = (digitalRead(sensor_3) == LOW);
    }

    // --- Кнопки ---
    bool startPressed()    { return getPressed(startBtn); }
    bool stopPressed()     { return getPressed(stopBtn); }
    bool modeChanged()     { return getPressed(modeBtn); }
    bool singleBlockPressed() { return getPressed(singleBlockBtn); }

    // --- Датчики ---
    // Датчик 1: наявність баночки під соплом розливу фарби
    bool isSensor1Active() { return sensor1State; }
    // Датчик 2: наявність баночки під пресом закривання кришки
    bool isSensor2Active() { return sensor2State; }
    // Датчик 3: спайка баночок готова до здвигання
    bool isSensor3Active() { return sensor3State; }

private:
    static constexpr unsigned long debounceDelay = 50;

    ButtonState startBtn, stopBtn, modeBtn, singleBlockBtn;
    bool sensor1State = false;
    bool sensor2State = false;
    bool sensor3State = false;

    void updateButton(uint8_t pin, ButtonState& btn) {
        bool reading = (digitalRead(pin) == LOW);
        if (reading != btn.last) {
            btn.lastChange = millis();
        }
        if ((millis() - btn.lastChange) > debounceDelay) {
            if (reading != btn.current) {
                btn.current = reading;
                if (btn.current) btn.pressedEvent = true;
            }
        }
        btn.last = reading;
    }

    bool getPressed(ButtonState& btn) {
        if (btn.pressedEvent) {
            btn.pressedEvent = false;
            return true;
        }
        return false;
    }
};