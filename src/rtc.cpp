#include "rtc.h"

RTCManager::RTCManager() {
    // Конструктор пустой, инициализация в begin()
}

bool RTCManager::begin() {
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

DateTime RTCManager::getDateTime() {
    return rtc.now();
}

void RTCManager::setDateTime(const DateTime &dt) {
    rtc.adjust(dt);
}

uint8_t RTCManager::getHour() {
    return rtc.now().hour();
}

uint8_t RTCManager::getMinute() {
    return rtc.now().minute();
}

uint8_t RTCManager::getSecond() {
    return rtc.now().second();
}

uint8_t RTCManager::getDay() {
    return rtc.now().day();
}

uint8_t RTCManager::getMonth() {
    return rtc.now().month();
}

uint16_t RTCManager::getYear() {
    return rtc.now().year();
}
