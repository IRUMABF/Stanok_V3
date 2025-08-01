#include <Arduino.h>
#include "pinout.h"
#include "config.h"
#include "pnevmatik_time.h"
#include "vacuumValve.h"
#include "pneumatic_valve.h"


// Створення об'єктів для кожного клапана
PneumaticValve valve1(PNEUMATIC_1_PIN);
PneumaticValve valve2(PNEUMATIC_2_PIN);
PneumaticValve valve3(PNEUMATIC_3_PIN);
PneumaticValve valve4(PNEUMATIC_4_PIN);
PneumaticValve valve5(PNEUMATIC_5_PIN); 
PneumaticValve valve6(PNEUMATIC_6_PIN); 
PneumaticValve valve7(PNEUMATIC_7_PIN);
PneumaticValve valve8(PNEUMATIC_8_PIN);
PneumaticValve valve9(PNEUMATIC_9_PIN);
PneumaticValve valve10(PNEUMATIC_10_PIN);
PneumaticValve valve11(PNEUMATIC_11_PIN);
PneumaticValve valve12(PNEUMATIC_12_PIN);
PneumaticValve valve13(PNEUMATIC_13_PIN);
PneumaticValve valve14(PNEUMATIC_14_PIN);

void setup() {
  Serial.begin(9600); // put your setup code here, to run once:
  vacuumValveInit();// Initialize vacuum valve
  setVacuumValve(VALVE_POS_1);// Set initial position of the vacuum valve
  Serial.println("Setup complete");

  valve1.begin();
  valve2.begin();
  valve3.begin();
  valve4.begin();
  valve5.begin();
  valve6.begin();
  valve7.begin();
  valve8.begin();
  valve9.begin();
  valve10.begin();
  valve11.begin();
  valve12.begin();
  valve13.begin();
  valve14.begin();
  // ...
  valve1.on();   // Відкрити клапан 1
  valve2.off();  // Закрити клапан 2
  valve3.on();   // Відкрити клапан 3
}

void loop() {
  // Керування клапанами
  //if (умова) valve1.on();
  //if (інша_умова) valve1.off();
  // put your main code here, to run repeatedly:
}