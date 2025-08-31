#pragma once
#include <Arduino.h>
#include "pinout.h"

// Клас для роботи з датчиками (тільки утиліти, без дублювання ініціалізації)
class SensorUtils {
public:
    // Видаляємо дублювання ініціалізації - це робить клас Controls
    
    // Отримання інформації про всі датчики (використовуємо зовнішні значення)
    static void getSensorStatus(bool s1, bool s2, bool s3, bool& out_s1, bool& out_s2, bool& out_s3) {
        out_s1 = s1;
        out_s2 = s2;
        out_s3 = s3;
    }
    
    // Виведення статусу датчиків в Serial
    static void printSensorStatus(bool s1, bool s2, bool s3) {
        Serial.print("Sensors: S1=");
        Serial.print(s1 ? "ON" : "OFF");
        Serial.print(" S2=");
        Serial.print(s2 ? "ON" : "OFF");
        Serial.print(" S3=");
        Serial.println(s3 ? "ON" : "OFF");
    }
    
    // Детальна відладка датчиків
    static void debugSensors(bool s1, bool s2, bool s3) {
        Serial.println("=== SENSOR DEBUG ===");
        Serial.print("Sensor 1 (Paint dispense): ");
        Serial.println(s1 ? "ACTIVE" : "INACTIVE");
        Serial.print("Sensor 2 (Cap press): ");
        Serial.println(s2 ? "ACTIVE" : "INACTIVE");
        Serial.print("Sensor 3 (Spice shift): ");
        Serial.println(s3 ? "ACTIVE" : "INACTIVE");
        Serial.println("===================");
    }
    
    // Чекання спрацювання конкретного датчика (використовуємо зовнішню функцію читання)
    static bool waitForSensor(bool (*readSensor)(), unsigned long timeout = 30000) {
        unsigned long startTime = millis();
        
        while (millis() - startTime < timeout) {
            bool sensorActive = readSensor();
            if (sensorActive) {
                Serial.println("Sensor triggered!");
                return true;
            }
            delay(10);
        }
        
        Serial.println("Timeout waiting for sensor");
        return false;
    }
    
    // Чекання вимкнення конкретного датчика
    static bool waitForSensorOff(bool (*readSensor)(), unsigned long timeout = 30000) {
        unsigned long startTime = millis();
        
        while (millis() - startTime < timeout) {
            bool sensorActive = readSensor();
            if (!sensorActive) {
                Serial.println("Sensor released!");
                return true;
            }
            delay(10);
        }
        
        Serial.println("Timeout waiting for sensor OFF");
        return false;
    }
    
    // Тестування всіх датчиків (використовуємо зовнішні функції читання)
    static void testAllSensors(bool (*readSensor1)(), bool (*readSensor2)(), bool (*readSensor3)()) {
        Serial.println("=== SENSOR TEST ===");
        Serial.println("Testing all sensors...");
        
        for (int i = 0; i < 10; i++) {
            bool s1 = readSensor1();
            bool s2 = readSensor2();
            bool s3 = readSensor3();
            
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