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

// Создание экземпляров наших менеджеров
SensorManager sensorManager;
ClimateManager climateManager;
DisplayManager displayManager;
MenuManager menuManager;
FSM fsm;
OzoneManager ozoneManager;
RTCManager rtcManager;
EEPROMManager eepromManager;

void setup() {
    // Инициализация последовательной связи
    Serial.begin(960);
    
    // Инициализация всех подсистем
    sensorManager.init();
    displayManager.init();
    menuManager.init();
    fsm.init();
    ozoneManager.init();
    rtcManager.init();
    eepromManager.init();
    
    // Инициализация менеджера климата после сенсоров
    climateManager.init(&sensorManager);
    
    // Отображение стартового сообщения
    displayManager.displayStartupMessage();
    
    Serial.println("Cellar Controller initialized");
}

void loop() {
    // Обновление всех подсистем
    sensorManager.updateSensors();
    climateManager.update();
    ozoneManager.update();
    rtcManager.update();
    fsm.update();
    
    // Обновление дисплея
    displayManager.update();
    
    // Обработка меню
    menuManager.handleMenu();
    
    // Небольшая задержка, чтобы не перегружать процессор
    delay(10);
}