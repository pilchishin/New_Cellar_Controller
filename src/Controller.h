#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include "Types.h"
#include "Config.h"
#include "SensorManager.h"
#include "RelayManager.h"
#include "TimeManager.h"
#include "AppEEPROM.h"

// Предварительное объявление, чтобы избежать циклической зависимости
class DisplayUI; 

class Controller {
private:
    SystemState currentState;
    ErrorCode currentError;

    // Уставки климата
    float targetTemp;
    float targetRh;

    // Калибровочные данные
    CalibrationData calib;

    // Статистика
    SystemStatistics stats;
    unsigned long lastStatsUpdate;
    unsigned long lastEEPROMSave;

    // EEPROM
    AppEEPROM storage;
    
    // Ссылки на модули
    SensorManager* sensors;
    RelayManager* relays;
    TimeManager* rtc;
    DisplayUI* ui;

    // Переменные логики
    unsigned long stateTimer;        // Timer for ozone phases and manual modes
    uint16_t manualTimer;            // Manual mode duration in minutes
    unsigned long retryOzoneTimer;   // Таймер для повтора при запрете (30 мин)
    bool ozoneInhibitedToday;        // Флаг, что сегодня попытка уже была

    // Внутренние методы обработки состояний
    void handleAutoClimate();
    void handleOzoneCycle();
    void handleManualModes();
    
    // Проверка критических условий
    void checkCriticalErrors();
    
    // Переход между состояниями
    void changeState(SystemState newState);

public:
    Controller(SensorManager* s, RelayManager* r, TimeManager* t);
    
    void setUI(DisplayUI* u) { ui = u; } // Установка UI после его инициализации
    
    void init();
    void tick(); // Основной цикл логики

    // Управление из UI
    void resetError();
    void startManualFan(uint16_t minutes);
    void startManualOzone(uint16_t minutes);
    
    // Геттеры для UI
    SystemState getState() const { return currentState; }
    ErrorCode getError() const { return currentError; }
    RelayManager* getRelayManager() const { return relays; }

    float getTargetTemp() const { return targetTemp; }
    float getTargetRh() const { return targetRh; }
    void setTargetTemp(float t);
    void setTargetRh(float h);

    CalibrationData getCalibration() const { return calib; }
    void setCalibration(const CalibrationData& data);

    SystemStatistics getStats() const { return stats; }
    void resetStats();
};

#endif
