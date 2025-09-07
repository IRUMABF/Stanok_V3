/*
    // --- Sensor debugging (every 2 seconds) ---
    unsigned long now = millis();
    if (now - lastDebugTime > DEBUG_INTERVAL) {
        lastDebugTime = now;

        if (DEBUG_LEVEL == 0) {
            // Output nothing
        } else if (DEBUG_LEVEL == 1) {
            // Basic information
            Serial.println("=== STATUS (BASIC) ===");
            Serial.print("Machine status: ");
            if (machinePaused) {
                Serial.println("PAUSED");
            } else if (machineRunning) {
                Serial.println("RUNNING");
            } else {
                Serial.println("STOPPED");
            }

            Serial.print("Operation mode: ");
            Serial.println("AUTO");

            Serial.print("Dispense mode: ");
            Serial.println(dispenseMode ? "ALL JARS" : "ONE JAR");

        
        Serial.print("Jar counter: ");
        Serial.print(jarCounter);
        Serial.print("/");
        Serial.println(JARS_IN_SET);
        
        Serial.print("Spice set counter: ");
        Serial.print(spiceSetCounter);
        Serial.print("/");
        Serial.println(SPICE_SETS_PER_PACKAGE);
        
        Serial.print("Conveyor status: ");
        Serial.println(conveyor.isRunning() ? "RUNNING" : "STOPPED");
        
        Serial.print("Active cycles: ");
        if (paintCycleActive) Serial.print("PAINT ");
        if (capCycleActive) Serial.print("CAPS ");
        if (!paintCycleActive && !capCycleActive) Serial.print("NONE");
        Serial.println();
        
        // Timer information
        if (paintCycleActive) {
            Serial.print("Paint cycle time: ");
            Serial.print(MS_TO_SECONDS(millis() - paintCycleStartTime));
            Serial.print("/");
            Serial.print(MS_TO_SECONDS(PAINT_CYCLE_TOTAL_TIME));
            Serial.println(" seconds");
        }
        
        if (capCycleActive) {
            Serial.print("Cap cycle time: ");
            Serial.print(MS_TO_SECONDS(millis() - capCycleStartTime));
            Serial.print("/");
            Serial.print(MS_TO_SECONDS(CAP_CYCLE_TOTAL_TIME));
            Serial.println(" seconds");
        }
        
        
        Serial.println("=======================");
        } else { // DEBUG_LEVEL >= 2 — full detailed debugging
            // Output detailed information to Serial
            Serial.println("=== STATUS UPDATE ===");
            Serial.print("Machine status: ");
            if (machinePaused) {
                Serial.println("PAUSED");
            } else if (machineRunning) {
                Serial.println("RUNNING");
            } else {
                Serial.println("STOPPED");
            }
            
            Serial.print("Operation mode: ");
            Serial.println("AUTO");
            
            Serial.print("Dispense mode: ");
            Serial.println(dispenseMode ? "ALL JARS" : "ONE JAR");
            
        
        
        Serial.print("Jar counter: ");
        Serial.print(jarCounter);
        Serial.print("/");
        Serial.println(JARS_IN_SET);
        
        Serial.print("Spice set counter: ");
        Serial.print(spiceSetCounter);
        Serial.print("/");
        Serial.println(SPICE_SETS_PER_PACKAGE);
        
        Serial.print("Conveyor status: ");
        Serial.println(conveyor.isRunning() ? "RUNNING" : "STOPPED");
        
        Serial.print("Conveyor overrun: ");
        Serial.println(conveyor.isDociagActive() ? "ACTIVE" : "INACTIVE");
        
        Serial.println("--- SENSOR STATUS ---");
        Serial.print("Sensor 1 (Paint dispensing): ");
        Serial.println(sensor1 ? "ACTIVE - Jar under nozzle" : "INACTIVE - No jar");
        
        Serial.print("Sensor 2 (Cap press): ");
        Serial.println(sensor2 ? "ACTIVE - Jar under press" : "INACTIVE - No jar");
        
        Serial.print("Sensor 3 (Spice shift): ");
        Serial.println(sensor3 ? "ACTIVE - Spice set ready" : "INACTIVE - Not ready");
        
        Serial.println("--- BUTTON STATUS ---");
        Serial.print("START button: ");
        Serial.println(startBtn ? "PRESSED" : "RELEASED");
        
        Serial.print("STOP button: ");
        Serial.println(stopBtn ? "PRESSED" : "RELEASED");
        
        Serial.print("MODE button: ");
        Serial.println(modeBtn ? "PRESSED" : "RELEASED");
        
        // SINGLE STEP button removed
        
        Serial.println("--- VALVE STATUS ---");
        Serial.print("Valve 1: ");
        Serial.println(valve1.isOn() ? "ON" : "OFF");
        Serial.print("Valve 2: ");
        Serial.println(valve2.isOn() ? "ON" : "OFF");
        Serial.print("Valve 3: ");
        Serial.println(valve3.isOn() ? "ON" : "OFF");
        
        Serial.println("--- CYCLE STATUS ---");
        Serial.print("Paint cycle: ");
        Serial.println(paintCycleActive ? "ACTIVE" : "INACTIVE");
        Serial.print("Cap closing cycle: ");
        Serial.println(capCycleActive ? "ACTIVE" : "INACTIVE");
        
        // Timer information for detailed debug
        Serial.println("--- TIMER STATUS ---");
        if (waitingForSensor1) {
            Serial.print("Sensor 1 wait time: ");
            Serial.print(MS_TO_SECONDS(millis() - sensor1StartTime));
            Serial.print("/");
            Serial.print(MS_TO_SECONDS(SENSOR_TIMEOUT_TIME));
            Serial.println(" seconds");
        }
        
        if (waitingForSensor2) {
            Serial.print("Sensor 2 wait time: ");
            Serial.print(MS_TO_SECONDS(millis() - sensor2StartTime));
            Serial.print("/");
            Serial.print(MS_TO_SECONDS(SENSOR_TIMEOUT_TIME));
            Serial.println(" seconds");
        }
        

        
        if (paintCycleActive) {
            Serial.print("Paint cycle time: ");
            Serial.print(MS_TO_SECONDS(millis() - paintCycleStartTime));
            Serial.print("/");
            Serial.print(MS_TO_SECONDS(PAINT_CYCLE_TOTAL_TIME));
            Serial.println(" seconds");
        }
        
        if (capCycleActive) {
            Serial.print("Cap cycle time: ");
            Serial.print(MS_TO_SECONDS(millis() - capCycleStartTime));
            Serial.print("/");
            Serial.print(MS_TO_SECONDS(CAP_CYCLE_TOTAL_TIME));
            Serial.println(" seconds");
        }
        
        
        Serial.println("===================");
        }
    }
*/