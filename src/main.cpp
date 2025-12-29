#include <Arduino.h>
#include "sensors.h"
#include "climate.h"
#include "display.h"
#include "menu.h"
#include "fsm.h"
#include "ozone.h"
#include "rtc.h"
#include "eeprom.h"
#include "utils.h"

// Create instances of our managers
SensorManager sensorManager;
ClimateManager climateManager;
DisplayManager displayManager;
MenuManager menuManager;
FSM fsm;
OzoneManager ozoneManager;
RTCManager rtcManager;
EEPROMManager eepromManager;

void setup() {
    // Initialize serial communication
    Serial.begin(9600);
    
    // Initialize all subsystems
    sensorManager.init();
    displayManager.init();
    menuManager.init();
    fsm.init();
    ozoneManager.init();
    rtcManager.init();
    eepromManager.init();
    
    // Initialize climate manager after sensors
    climateManager.init(&sensorManager);
    
    // Display startup message
    displayManager.displayStartupMessage();
    
    Serial.println("Cellar Controller initialized");
}

void loop() {
    // Update all subsystems
    sensorManager.updateSensors();
    climateManager.update();
    ozoneManager.update();
    rtcManager.update();
    fsm.update();
    
    // Update display
    displayManager.update();
    
    // Handle menu
    menuManager.handleMenu();
    
    // Small delay to prevent overwhelming the processor
    delay(10);
}