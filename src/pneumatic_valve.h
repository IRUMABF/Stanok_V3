#ifndef PNEUMATIC_VALVE_H
#define PNEUMATIC_VALVE_H

#include <Arduino.h>

class PneumaticValve {
  public:
    PneumaticValve(uint8_t pin) : _pin(pin) {}

    void begin() {
      pinMode(_pin, OUTPUT);
      off();
    }

    void on() {
      digitalWrite(_pin, HIGH);
      _state = true;
      _autoOff = false;
    }

    void off() {
      digitalWrite(_pin, LOW);
      _state = false;
      _autoOff = false;
    }

    // Включити клапан на певний час (мс), потім автоматично вимкнути
    void onFor(unsigned long duration) {
      on();
      _autoOff = true;
      _offTime = millis() + duration;
      _pendingAction = 0; // 0 - після таймера вимкнути
    }

    // Вимкнути клапан на певний час (мс), потім автоматично увімкнути
    void offFor(unsigned long duration) {
      off();
      _autoOff = true;
      _offTime = millis() + duration;
      _pendingAction = 1; // 1 - після таймера увімкнути
    }

    // Викликати цю функцію в loop() для обслуговування таймера
    void update() {
      if (_autoOff && millis() >= _offTime) {
        if (_pendingAction == 0) {
          off();
        } else if (_pendingAction == 1) {
          on();
        }
        _autoOff = false;
      }
    }

    void toggle() {
      if (_state) off();
      else on();
    }

    bool isOn() const {
      return _state;
    }

    uint8_t getPin() const {
      return _pin;
    }

  private:
    uint8_t _pin;
    bool _state = false;
    bool _autoOff = false;
    unsigned long _offTime = 0;
    uint8_t _pendingAction = 0; // 0 - off після onFor, 1 - on після offFor
};

#endif