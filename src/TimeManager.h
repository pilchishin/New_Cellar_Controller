#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <RTClib.h> // Библиотека Adafruit RTClib
#include "Types.h"

class TimeManager {
 private:
  RTC_DS3231 rtc_;
  DateTime current_time_;

  // Настройки расписания озонирования (по умолчанию раз в неделю)
  uint8_t schedule_day_of_week_;  // 0 - Воскресенье, 1 - Понедельник, ... 6 - Суббота
  uint8_t schedule_hour_;
  uint8_t schedule_minute_;

  // Переменная для защиты от повторного запуска в тот же день,
  // если цикл прервется из-за мороза на улице и FSM вернется в AUTO_CLIMATE.
  int8_t last_ozone_trigger_day_;

  // Состояние модуля
  bool rtc_valid_;
  uint8_t read_error_count_;
  unsigned long rtc_retry_timer_ = 0;
  static constexpr unsigned long kRtcRetryInterval = 30000UL;

 public:
  TimeManager();

  // Инициализация модуля и начальная проверка валидности времени
  void Init();

  // Обновление текущего времени
  void Update();

  // Возвращает код ошибки (если связи нет после 3 попыток или время сбито)
  ErrorCode CheckErrors();

  /**
   * @brief Проверка наступления времени планового озонирования.
   */
  bool IsOzoneTimeScheduled();

  // Геттеры для UI
  DateTime GetTime() const { return current_time_; }

  // Сеттеры для изменения расписания из меню (Service menu)
  void SetOzoneSchedule(uint8_t day, uint8_t hour, uint8_t minute);
};

#endif