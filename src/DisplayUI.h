#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Controller.h"
#include "SensorManager.h"
#include "TimeManager.h"

// Состояния меню
enum class MenuPage {
    HOME_SCREEN,    // Главная: T/H, цели и статусы реле
    STATUS_IN,      // Датчики внутри (детально: AH, точка росы)
    STATUS_OUT,     // Датчики снаружи (детально: AH)
    SET_TEMP,       // Установка целевой T
    SET_HUM,        // Установка целевой RH
    MANUAL_MODES,   // Ручной запуск FAN/OZONE
    ERROR_LOG,      // Просмотр и сброс ошибок
    STATS,          // Статистика работы
    CALIB_BME_T,    // Калибровка BME T
    CALIB_BME_H,    // Калибровка BME H
    CALIB_HTU_T,    // Калибровка HTU T
    CALIB_HTU_H,    // Калибровка HTU H
    CALIB_DS_T      // Калибровка DS T
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
    unsigned long lastBtnAction; // Таймер повтора для UP/DOWN
    unsigned long menuBtnTimer;
    bool menuBtnPressed;

    // Временные сообщения на экране
    unsigned long messageTimer;
    const char* tempMessage;
    
    // Таймер подсветки
    unsigned long lastActivityTime;
    bool backlightOn;

    void handleButtons();
    void drawPage();
    void updateBacklight();

    // Вспомогательные методы отрисовки
    void drawHomeScreen();
    void drawStatusIn();
    void drawStatusOut();
    void drawSetTemp();
    void drawSetHum();
    void drawManualModes();
    void drawCalibPage(const char* label, float value, bool isTemp);
    void drawStats();
    void drawErrorLog();

public:
    DisplayUI(Controller* c, SensorManager* s, TimeManager* t);
    void init();
    void reinit(); // Повторная инициализация LCD после сбоя I2C
    void update(); // Вызывается в основном loop()
    
    bool isBacklightOn() const { return backlightOn; }
};

#endif