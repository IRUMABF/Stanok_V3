#pragma once
#include <Arduino.h>
#include "pinout.h"

// Клас для роботи з датчиками
class SensorUtils {
public:
    // Ініціалізація датчиків
    static void initSensors() {
        pinMode(sensor_1, INPUT_PULLUP);
        pinMode(sensor_2, INPUT_PULLUP);
        pinMode(sensor_3, INPUT_PULLUP);
        Serial.println("Sensors initialized:");
        Serial.println("- Sensor 1 (Paint dispense): Pin " + String(sensor_1));
        Serial.println("- Sensor 2 (Cap press): Pin " + String(sensor_2));
        Serial.println("- Sensor 3 (Spice shift): Pin " + String(sensor_3));
    }
    
    // Читання стану датчиків
    static bool readSensor1() {
        return !digitalRead(sensor_1); // Інвертуємо через INPUT_PULLUP
    }
    
    static bool readSensor2() {
        return !digitalRead(sensor_2);
    }
    
    static bool readSensor3() {
        return !digitalRead(sensor_3);
    }
    
    // Отримання інформації про всі датчики
    static void getSensorStatus(bool& s1, bool& s2, bool& s3) {
        s1 = readSensor1();
        s2 = readSensor2();
        s3 = readSensor3();
    }
    
    // Виведення статусу датчиків в Serial
    static void printSensorStatus() {
        bool s1, s2, s3;
        getSensorStatus(s1, s2, s3);
        
        Serial.print("Sensors: S1=");
        Serial.print(s1 ? "ON" : "OFF");
        Serial.print(" S2=");
        Serial.print(s2 ? "ON" : "OFF");
        Serial.print(" S3=");
        Serial.println(s3 ? "ON" : "OFF");
    }
    
    // Детальна відладка датчиків
    static void debugSensors() {
        bool s1, s2, s3;
        getSensorStatus(s1, s2, s3);
        
        Serial.println("=== SENSOR DEBUG ===");
        Serial.print("Sensor 1 (Paint dispense): ");
        Serial.println(s1 ? "ACTIVE" : "INACTIVE");
        Serial.print("Sensor 2 (Cap press): ");
        Serial.println(s2 ? "ACTIVE" : "INACTIVE");
        Serial.print("Sensor 3 (Spice shift): ");
        Serial.println(s3 ? "ACTIVE" : "INACTIVE");
        Serial.println("===================");
    }
    
    // Чекання спрацювання конкретного датчика
    static bool waitForSensor(uint8_t sensorPin, unsigned long timeout = 30000) {
        unsigned long startTime = millis();
        
        while (millis() - startTime < timeout) {
            bool sensorActive = !digitalRead(sensorPin);
            if (sensorActive) {
                Serial.print("Sensor on pin ");
                Serial.print(sensorPin);
                Serial.println(" triggered!");
                return true;
            }
            delay(10);
        }
        
        Serial.print("Timeout waiting for sensor on pin ");
        Serial.println(sensorPin);
        return false;
    }
    
    // Чекання вимкнення конкретного датчика
    static bool waitForSensorOff(uint8_t sensorPin, unsigned long timeout = 30000) {
        unsigned long startTime = millis();
        
        while (millis() - startTime < timeout) {
            bool sensorActive = !digitalRead(sensorPin);
            if (!sensorActive) {
                Serial.print("Sensor on pin ");
                Serial.print(sensorPin);
                Serial.println(" released!");
                return true;
            }
            delay(10);
        }
        
        Serial.print("Timeout waiting for sensor OFF on pin ");
        Serial.println(sensorPin);
        return false;
    }
    
    // Тестування всіх датчиків
    static void testAllSensors() {
        Serial.println("=== SENSOR TEST ===");
        Serial.println("Testing all sensors...");
        
        for (int i = 0; i < 10; i++) {
            bool s1, s2, s3;
            getSensorStatus(s1, s2, s3);
            
            Serial.print("Test ");
            Serial.print(i + 1);
            Serial.print(": S1=");
            Serial.print(s1 ? "1" : "0");
            Serial.print(" S2=");
            Serial.print(s2 ? "1" : "0");
            Serial.print(" S3=");
            Serial.println(s3 ? "1" : "0");
            
            delay(500);
        }
        
        Serial.println("Sensor test completed");
    }
}; 