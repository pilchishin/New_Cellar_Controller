#ifndef RTC_H
#define RTC_H

#include <Arduino.h>
#include <RTClib.h>

/**
 * @brief Класс для работы с RTC DS3231
 * 
 * Класс предоставляет функциональность для работы с часами реального времени DS3231
 */
class DS3231RTC {
public:
    /**
     * @brief Конструктор класса DS3231RTC
     */
    DS3231RTC();
    
    /**
     * @brief Инициализация RTC
     * @return true в случае успешной инициализации, false в противном случае
     */
    bool initialize();
    
    /**
     * @brief Чтение текущего времени
     * @return Объект DateTime с текущим временем
     */
    DateTime readTime();
    
    /**
     * @brief Установка времени
     * @param time Объект DateTime с временем для установки
     * @return true в случае успешной установки, false в противном случае
     */
    bool setTime(const DateTime& time);
    
    /**
     * @brief Проверка точности времени
     * @return true если время точное, false если есть расхождения
     */
    bool checkTimeAccuracy();
    
    /**
     * @brief Настройка будильника
     * @param time Время для установки будильника
     * @return true в случае успешной настройки, false в противном случае
     */
    bool scheduleAlarm(const DateTime& time);
    
    /**
     * @brief Проверка действительности времени
     * @return true если время действительное, false в противном случае
     */
    bool isTimeValid();

private:
    RTC_DS3231 rtc; ///< Объект RTC
    unsigned long lastSyncTime; ///< Время последней синхронизации
    bool timeValid; ///< Действительность времени
};

#endif // RTC_H