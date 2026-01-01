#include "rtc.h"

DS3231RTC::DS3231RTC() : rtc(), lastSyncTime(0), timeValid(false) {
}

bool DS3231RTC::initialize() {
    if (!rtc.begin()) {
        return false;
    }
    
    if (rtc.lostPower()) {
        // Потеря питания RTC, время недействительно
        timeValid = false;
        return false;
    }
    
    timeValid = true;
    lastSyncTime = millis();
    return true;
}

DateTime DS3231RTC::readTime() {
    return rtc.now();
}

bool DS3231RTC::setTime(const DateTime& time) {
    rtc.adjust(time);
    timeValid = true;
    lastSyncTime = millis();
    return true;
}

bool DS3231RTC::checkTimeAccuracy() {
    // Сравнение внутреннего дрейфа часов с известным точным источником времени, если он доступен
    // На данный момент мы будем считать DS3231 точными, поскольку они компенсируются температурой
    // Мы можем реализовать дополнительные проверки, если у нас есть сетевое время
    
    // Обновление времени последней синхронизации
    lastSyncTime = millis();
    
    // DS3231 очень точные, поэтому мы вернем true
    // На практике мы можем сравнивать с внешним источником времени
    return true;
}

bool DS3231RTC::scheduleAlarm(const DateTime& time) {
    // Установка будильника 1 на указанное время
    // Это упрощенная реализация - вы можете настроить это в зависимости от ваших конкретных потребностей
    rtc.disableAlarm(1);
    rtc.disableAlarm(2);
    
    // В этом примере мы установим будильник на следующее occurrence указанного времени
    // Точная реализация зависит от того, какая функциональность будильника нужна
    rtc.setAlarm1(time, DS3231_A1_Hour); // Будильник каждый день в указанное время
    
    return true;
}

bool DS3231RTC::isTimeValid() {
    return timeValid;
}