#ifndef RTCLIB_H
#define RTCLIB_H
#include <stdint.h>
#include "Arduino.h"

class DateTime {
public:
    DateTime() : y(2023), m(1), d(1), hh(0), mm(0), ss(0) {}
    DateTime(uint32_t t) {
        y = 2023; m = 1; d = 1;
        hh = (t / 3600) % 24;
        mm = (t / 60) % 60;
        ss = t % 60;
    }
    DateTime(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
        : y(year), m(month), d(day), hh(hour), mm(min), ss(sec) {}
    DateTime(const __FlashStringHelper* date, const __FlashStringHelper* time) : y(2023), m(1), d(1), hh(0), mm(0), ss(0) {}

    uint16_t year() const { return y; }
    uint8_t month() const { return m; }
    uint8_t day() const { return d; }
    uint8_t hour() const { return hh; }
    uint8_t minute() const { return mm; }
    uint8_t second() const { return ss; }
    uint8_t dayOfTheWeek() const { return 0; } // Mock: Sunday
    uint32_t unixtime() const { return hh * 3600 + mm * 60 + ss; } // Mock unixtime within a day
private:
    uint16_t y; uint8_t m, d, hh, mm, ss;
};

class RTC_DS3231 {
public:
    bool begin() { return true; }
    bool lostPower() { return false; }
    void adjust(const DateTime& dt) {}
    DateTime now();
};
#endif
