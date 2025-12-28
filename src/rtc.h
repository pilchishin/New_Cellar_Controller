#ifndef RTC_H
#define RTC_H

#include "config.h"
#include <RTClib.h>

class RTCManager {
public:
    RTCManager();
    void init();
    void update();
    
    bool setTime(int hour, int minute, int second, int day, int month, int year);
    bool setTimeFromSensor();
    
    String getTimeString();
    String getDateString();
    DateTime now();
    
    void enableAlarm();
    void disableAlarm();
    bool isAlarmEnabled();
    
    void setAlarmTime(int hour, int minute);
    bool isAlarmTime();
    
    bool isTimeSet();
    void syncWithSensor();
    
private:
    RTC_DS3231* rtc;
    bool alarmEnabled;
    int alarmHour;
    int alarmMinute;
};

#endif // RTC_H