#include "TimeManager.h"

TimeManager::TimeManager() {
  // По ТЗ: запуск раз в неделю в 02:00.
  // Пусть по умолчанию это будет Воскресенье (0 в RTClib)
  schedule_day_of_week_ = 0;
  schedule_hour_ = 2;
  schedule_minute_ = 0;

  last_ozone_trigger_day_ = -1;  // Еще не запускался
  rtc_valid_ = false;
  read_error_count_ = 0;
}

void TimeManager::Init() {
#ifdef DEBUG
  Serial.println(F("Init DS3231 RTC..."));
#endif

  // Инициализация шины и поиск устройства
  if (!rtc_.begin()) {
#ifdef DEBUG
    Serial.println(F("Couldn't find RTC"));
#endif
    rtc_valid_ = false;
    return;
  }

  // Проверка на то, сбрасывалось ли питание RTC (села батарейка CR2032)
  if (rtc_.lostPower()) {
        #ifdef DEBUG
        Serial.println(F("RTC lost power, time is invalid!"));
        #endif
        // Если батарейка села, время недействительно. 
        // В реальном устройстве здесь можно установить время компиляции прошивки:
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // Но по ТЗ это критическая ошибка, требующая внимания.
    rtc_valid_ = false;
    return;
  }

  // Если связь есть и питание не пропадало — делаем первое чтение
  current_time_ = rtc_.now();

  // Проверка валидности (расхождение > 1 мин при старте).
    // Поскольку Arduino Nano не имеет интернета (NTP), лучший способ проверить 
  // "адекватность" времени — сравнить его с датой компиляции прошивки.
  // Если время на часах меньше времени компиляции прошивки, значит часы сбиты.
  DateTime compiled_time(F(__DATE__), F(__TIME__));
  if (current_time_.unixtime() < compiled_time.unixtime()) {
#ifdef DEBUG
    Serial.println(F("RTC time is older than firmware build time! ERROR."));
#endif
    rtc_valid_ = false;
    return;
  }

  rtc_valid_ = true;
  read_error_count_ = 0;
}

void TimeManager::Update() {
  // Если модуль помечен как неисправный, пытаемся восстановиться каждые 30 секунд
  if (!rtc_valid_) {
    if (millis() - rtc_retry_timer_ < kRtcRetryInterval) {
      return;
    }

#ifdef DEBUG
    Serial.println(F("RTC Recovery Attempt..."));
#endif

    // Пытаемся переинициализировать RTC
    if (rtc_.begin()) {
      DateTime temp = rtc_.now();
      // Год >= 2020 считается валидным для восстановления
      if (temp.year() >= 2020) {
        rtc_valid_ = true;
        read_error_count_ = 0;
#ifdef DEBUG
        Serial.println(F("RTC Recovered successfully."));
#endif
      }
    }

    if (!rtc_valid_) {
      rtc_retry_timer_ = millis();
      return;
    }
  }

  // Пытаемся прочитать время. Библиотека RTClib использует Wire.requestFrom.
  // Если устройство на шине зависло, может вернуться кривая дата.
  DateTime temp_time = rtc_.now();

  // Простая эвристика проверки успешного чтения I2C (обычно при сбое возвращается 2000 год)
  // Либо можно проверять статус шины Wire, но RTClib скрывает это.
  // Если год < 2023, считаем чтение ошибочным.
  if (temp_time.year() < 2023) {
    read_error_count_++;
#ifdef DEBUG
    Serial.print(F("RTC Read Error. Count: "));
    Serial.println(read_error_count_);
#endif

    if (read_error_count_ >= 3) {
      rtc_valid_ = false;  // Три ошибки подряд — критический отказ
      rtc_retry_timer_ = millis();
    }
  } else {
    // Успешное чтение
    current_time_ = temp_time;
    read_error_count_ = 0;  // Сбрасываем счетчик ошибок
    rtc_valid_ = true;
  }
}

ErrorCode TimeManager::CheckErrors() {
  if (!rtc_valid_ || read_error_count_ >= 3) {
    return ErrorCode::kRtcFail;
  }
  return ErrorCode::kNone;
}

/**
 * @brief Проверка наступления времени планового озонирования.
 * Озонирование запускается только если часы исправны.
 * Для предотвращения повторных пусков в ту же минуту используется флаг последнего дня пуска.
 */
bool TimeManager::IsOzoneTimeScheduled() {
  if (!rtc_valid_) return false;

  // Сравнение текущего времени с установленным расписанием
  if (current_time_.dayOfTheWeek() == schedule_day_of_week_ &&
      current_time_.hour() == schedule_hour_ &&
      current_time_.minute() == schedule_minute_) {
    // Защита от дребезга триггера:
    // Если сегодня цикл уже запускался, игнорируем совпадение.
    if (last_ozone_trigger_day_ != current_time_.dayOfTheWeek()) {
      // Фиксируем, что сегодня мы цикл запустили
      last_ozone_trigger_day_ = current_time_.dayOfTheWeek();

#ifdef DEBUG
      Serial.println(F("OZONE SCHEDULE TRIGGERED!"));
#endif

      return true;
    }
  }

  return false;
}

void TimeManager::SetOzoneSchedule(uint8_t day, uint8_t hour, uint8_t minute) {
  schedule_day_of_week_ = day % 7;  // Защита от выхода за границы 0-6
  schedule_hour_ = hour % 24;
  schedule_minute_ = minute % 60;
}