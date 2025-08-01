#ifndef VACUUM_VALVE_H
#define VACUUM_VALVE_H

#include <Arduino.h>
#include "pinout.h"

enum VacuumValvePosition {
    VALVE_POS_1 = 1, // IN1=1, IN2=0
    VALVE_POS_2 = 2, // IN1=0, IN2=1
    VALVE_POS_3 = 3  // IN1=1, IN2=1
};

inline void vacuumValveInit() {
    pinMode(vacuumValvePin1, OUTPUT);
    pinMode(vacuumValvePin2, OUTPUT);
    // За замовчуванням вимкнено
    digitalWrite(vacuumValvePin1, LOW);
    digitalWrite(vacuumValvePin2, LOW);
}

inline void setVacuumValve(uint8_t position) {
    switch (position) {
        case VALVE_POS_1:
            digitalWrite(vacuumValvePin1, HIGH);
            digitalWrite(vacuumValvePin2, LOW);
            break;
        case VALVE_POS_2:
            digitalWrite(vacuumValvePin1, LOW);
            digitalWrite(vacuumValvePin2, HIGH);
            break;
        case VALVE_POS_3:
            digitalWrite(vacuumValvePin1, HIGH);
            digitalWrite(vacuumValvePin2, HIGH);
            break;
        default:
            // Можна додати обробку помилки
            break;
    }
}

#endif