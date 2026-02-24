#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h> // Библиотека Adafruit RTClib
#include "Types.h"

class TimeManager {
private:
    RTC_DS3231 rtc;
    DateTime currentTime;

    // Настройки расписания озонирования (по умолчанию раз в неделю)
    uint8_t scheduleDayOfWeek; // 0 - Воскресенье, 1 - Понедельник, ... 6 - Суббота
    uint8_t scheduleHour;
    uint8_t scheduleMinute;

    // Переменная для защиты от повторного запуска в тот же день, 
    // если цикл прервется из-за мороза на улице и FSM вернется в AUTO_CLIMATE.
    int8_t lastOzoneTriggerDay; 

    // Состояние модуля
    bool rtcValid;
    uint8_t readErrorCount;

public:
    TimeManager();

    // Инициализация модуля и начальная проверка валидности времени
    void init();

    // Обновление текущего времени (вызывается в основном цикле раз в секунду или реже)
    void update();

    // Возвращает код ошибки (если связи нет после 3 попыток или время сбито)
    ErrorCode checkErrors();

    // Проверка, настало ли время запускать цикл озонирования
    bool isOzoneTimeScheduled();

    // Геттеры для UI
    DateTime getTime() const { return currentTime; }
    
    // Сеттеры для изменения расписания из меню (Service menu)
    void setOzoneSchedule(uint8_t day, uint8_t hour, uint8_t minute);
};

#endif