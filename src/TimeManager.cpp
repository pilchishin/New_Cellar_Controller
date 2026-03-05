#include "TimeManager.h"

TimeManager::TimeManager() {
    // По ТЗ: запуск раз в неделю в 02:00. 
    // Пусть по умолчанию это будет Воскресенье (0 в RTClib)
    scheduleDayOfWeek = 0; 
    scheduleHour = 2;
    scheduleMinute = 0;
    
    lastOzoneTriggerDay = -1; // Еще не запускался
    rtcValid = false;
    readErrorCount = 0;
}

void TimeManager::init() {
    #ifdef DEBUG
    Serial.println(F("Init DS3231 RTC..."));
    #endif

    // Инициализация шины и поиск устройства
    if (!rtc.begin()) {
        #ifdef DEBUG
        Serial.println(F("Couldn't find RTC"));
        #endif
        rtcValid = false;
        return;
    }

    // Проверка на то, сбрасывалось ли питание RTC (села батарейка CR2032)
    if (rtc.lostPower()) {
        #ifdef DEBUG
        Serial.println(F("RTC lost power, time is invalid!"));
        #endif
        // Если батарейка села, время недействительно. 
        // В реальном устройстве здесь можно установить время компиляции прошивки:
        // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // Но по ТЗ это критическая ошибка, требующая внимания.
        rtcValid = false;
        return;
    }

    // Если связь есть и питание не пропадало — делаем первое чтение
    currentTime = rtc.now();

    // Проверка валидности (расхождение > 1 мин при старте).
    // Поскольку Arduino Nano не имеет интернета (NTP), лучший способ проверить 
    // "адекватность" времени — сравнить его с датой компиляции прошивки.
    // Если время на часах меньше времени компиляции прошивки, значит часы сбиты.
    DateTime compiledTime(F(__DATE__), F(__TIME__));
    if (currentTime.unixtime() < compiledTime.unixtime()) {
        #ifdef DEBUG
        Serial.println(F("RTC time is older than firmware build time! ERROR."));
        #endif
        rtcValid = false;
        return;
    }

    rtcValid = true;
    readErrorCount = 0;
}

void TimeManager::update() {
    // Если модуль уже помечен как неисправный аппаратно, не пытаемся читать
    // (для восстановления потребуется перезагрузка контроллера)
    if (!rtcValid && readErrorCount >= 3) return;

    // Пытаемся прочитать время. Библиотека RTClib использует Wire.requestFrom.
    // Если устройство на шине зависло, может вернуться кривая дата.
    DateTime tempTime = rtc.now();

    // Простая эвристика проверки успешного чтения I2C (обычно при сбое возвращается 2000 год)
    // Либо можно проверять статус шины Wire, но RTClib скрывает это.
    // Если год < 2023, считаем чтение ошибочным.
    if (tempTime.year() < 2023) {
        readErrorCount++;
        #ifdef DEBUG
        Serial.print(F("RTC Read Error. Count: ")); Serial.println(readErrorCount);
        #endif

        if (readErrorCount >= 3) {
            rtcValid = false; // Три ошибки подряд — критический отказ
        }
    } else {
        // Успешное чтение
        currentTime = tempTime;
        readErrorCount = 0; // Сбрасываем счетчик ошибок
        rtcValid = true;
    }
}

ErrorCode TimeManager::checkErrors() {
    if (!rtcValid || readErrorCount >= 3) {
        return ErrorCode::RTC_FAIL;
    }
    return ErrorCode::NONE;
}

/**
 * @brief Проверка наступления времени планового озонирования.
 * Озонирование запускается только если часы исправны.
 * Для предотвращения повторных пусков в ту же минуту используется флаг последнего дня пуска.
 */
bool TimeManager::isOzoneTimeScheduled() {
    if (!rtcValid) return false;

    // Сравнение текущего времени с установленным расписанием
    if (currentTime.dayOfTheWeek() == scheduleDayOfWeek &&
        currentTime.hour() == scheduleHour &&
        currentTime.minute() == scheduleMinute) {
        
        // Защита от дребезга триггера:
        // Если сегодня цикл уже запускался, игнорируем совпадение.
        if (lastOzoneTriggerDay != currentTime.dayOfTheWeek()) {
            
            // Фиксируем, что сегодня мы цикл запустили
            lastOzoneTriggerDay = currentTime.dayOfTheWeek();
            
            #ifdef DEBUG
            Serial.println(F("OZONE SCHEDULE TRIGGERED!"));
            #endif
            
            return true;
        }
    }

    return false;
}

void TimeManager::setOzoneSchedule(uint8_t day, uint8_t hour, uint8_t minute) {
    scheduleDayOfWeek = day % 7; // Защита от выхода за границы 0-6
    scheduleHour = hour % 24;
    scheduleMinute = minute % 60;
}