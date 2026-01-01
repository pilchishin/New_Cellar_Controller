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
ClimateController climateController;
DisplayManager displayManager;
MenuManager menuManager;
FSM fsm;
OzoneController ozoneController;
RTCManager rtcManager;
EEPROMManager eepromManager;

void setup() {
    // Инициализация последовательной связи
    Serial.begin(9600);
    
    // Инициализация всех подсистем
    sensorManager.begin();
    displayManager.init();
    menuManager.init();
    fsm.init();
    ozoneController.init();
    rtcManager.begin();
    eepromManager.init();
    
    // Инициализация контроллера климата после сенсоров
    climateController.setSensorManager(&sensorManager);
    climateController.init();
    
    // Отображение стартового сообщения
    displayManager.displayStatusScreen();
    
    Serial.println("Cellar Controller initialized");
}

void loop() {
    // Обновление всех подсистем
    climateController.update();
    ozoneController.update();
    fsm.update();
    
    // Обновление дисплея
    displayManager.update();
    
    // Обработка меню
    menuManager.update();
    
    // Обновление опроса датчиков (логика с таймером находится внутри метода)
    sensorManager.update();
    
    // Небольшая задержка, чтобы не перегружать процессор
    delay(10);
}