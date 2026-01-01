#include "rtc.h"

RTC_DS3231::RTC_DS3231() {
    // Конструктор пустой, инициализация в begin()
}

bool RTC_DS3231::begin() {
    if (!rtc.begin()) {
        // RTC не найден
        return false;
    }

    // Если RTC остановлен, запускаем его
    if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }

    return true;
}

DateTime RTC_DS3231::getDateTime() {
    return rtc.now();
}

void RTC_DS3231::setDateTime(const DateTime &dt) {
    rtc.adjust(dt);
}

uint8_t RTC_DS3231::getHour() {
    return rtc.now().hour();
}

uint8_t RTC_DS3231::getMinute() {
    return rtc.now().minute();
}

uint8_t RTC_DS3231::getSecond() {
    return rtc.now().second();
}

uint8_t RTC_DS3231::getDay() {
    return rtc.now().day();
}

uint8_t RTC_DS3231::getMonth() {
    return rtc.now().month();
}

uint16_t RTC_DS3231::getYear() {
    return rtc.now().year();
}
