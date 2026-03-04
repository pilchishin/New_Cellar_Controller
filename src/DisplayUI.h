#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Controller.h"
#include "SensorManager.h"
#include "TimeManager.h"

// Состояния меню
enum class MenuPage {
    STATUS_IN,      // Датчики внутри
    STATUS_OUT,     // Датчики снаружи
    STATUS_RELAY,   // Состояние реле и FSM
    SET_TEMP,       // Установка целевой T
    SET_HUM,        // Установка целевой RH
    MANUAL_MODES,   // Ручной запуск FAN/OZONE
    CALIB_BME_T,    // Калибровка BME T
    CALIB_BME_H,    // Калибровка BME H
    CALIB_HTU_T,    // Калибровка HTU T
    CALIB_HTU_H,    // Калибровка HTU H
    CALIB_DS_T,     // Калибровка DS T
    ERROR_LOG       // Просмотр ошибок
};

class DisplayUI {
private:
    LiquidCrystal_I2C lcd;
    Controller* controller;
    SensorManager* sensors;
    TimeManager* rtc;

    MenuPage currentPage;
    
    // Переменные для кнопок
    unsigned long lastBtnCheck;
    unsigned long menuBtnTimer;
    bool menuBtnPressed;
    
    // Таймер подсветки
    unsigned long lastActivityTime;
    bool backlightOn;

    void handleButtons();
    void drawPage();
    void updateBacklight();

    // Вспомогательные методы отрисовки
    void drawStatusIn();
    void drawStatusOut();
    void drawRelayState();
    void drawSetTemp();
    void drawSetHum();
    void drawManualModes();
    void drawCalibPage(const char* label, float value, bool isTemp);
    void drawErrorLog();

public:
    DisplayUI(Controller* c, SensorManager* s, TimeManager* t);
    void init();
    void update(); // Вызывается в основном loop()
    
    bool isBacklightOn() const { return backlightOn; }
};

#endif