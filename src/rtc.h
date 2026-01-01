#ifndef RTC_H
#define RTC_H

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h>  // Библиотека для DS3231

/**
 * @brief Класс для работы с DS3231 RTC
 */
class RTC_DS3231 {
public:
    /**
     * @brief Конструктор
     */
    RTC_DS3231();

    /**
     * @brief Инициализация RTC
     * @return true если инициализация успешна, false если ошибка
     */
    bool begin();

    /**
     * @brief Получить текущие дату и время
     * @return Объект DateTime из библиотеки RTClib
     */
    DateTime getDateTime();

    /**
     * @brief Установить дату и время
     * @param dt Объект DateTime с нужными значениями
     */
    void setDateTime(const DateTime &dt);

    /**
     * @brief Получить текущие часы, минуты, секунды
     */
    uint8_t getHour();
    uint8_t getMinute();
    uint8_t getSecond();

    /**
     * @brief Получить текущие день, месяц, год
     */
    uint8_t getDay();
    uint8_t getMonth();
    uint16_t getYear();

private:
    RTC_DS3231_Class rtc;  // объект библиотеки RTClib
};

#endif // RTC_H