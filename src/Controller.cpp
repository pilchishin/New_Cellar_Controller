#include "Controller.h"
#include "DisplayUI.h"

/**
 * @brief Конструктор контроллера.
 * Загружает настройки из EEPROM и инициализирует таймеры.
 */
Controller::Controller(SensorManager* s, RelayManager* r, TimeManager* t)
    : sensors_(s), relays_(r), rtc_(t), ui_(nullptr) {
  current_state_ = SystemState::kIdle;
  current_error_ = ErrorCode::kNone;

  // Загрузка сохраненных данных
  PersistentData data;
  storage_.Load(data);

  target_temp_ = data.targetTemp;
  target_rh_ = data.targetRh;
  calib_ = data.calibration;
  stats_ = data.stats;

  // Применяем калибровку к менеджеру датчиков
  sensors_->SetCalibration(calib_);

  last_stats_update_ = millis();
  last_eeprom_save_ = millis();
  is_user_present_ = false;
  last_user_activity_time_ = 0;
  state_timer_ = 0;
  manual_timer_ = 0;
  retry_ozone_timer_ = 0;
}

/**
 * @brief Начало работы системы.
 */
void Controller::Init() {
  if (ui_ == nullptr) {
    // SetUI() не был вызван перед Init() — это ошибка программирования.
    // Останавливаем выполнение здесь, чтобы ошибку можно было заметить при разработке.
    while (true) {} // Сторожевой таймер (WDT) перезагрузит систему через 8 секунд.
  }
  ChangeState(SystemState::kAutoClimate);
}

/**
 * @brief Главный цикл обработки.
 */
void Controller::Tick() {
  UpdateStatistics();   // Обновление счетчиков наработки
  HandleStorage();      // Периодическое сохранение данных
  CheckSystemHealth();  // Контроль датчиков и ошибок
  UpdateUserPresence(); // Проверка активности пользователя
  ProcessStateMachine(); // Работа логики состояний (FSM)
}

/**
 * @brief Обновление моточасов оборудования.
 * Вызывается каждую итерацию, расчет производится раз в минуту.
 */
void Controller::UpdateStatistics() {
  unsigned long now = millis();
  if (now - last_stats_update_ >= 60000UL) {
    last_stats_update_ = now;
    stats_.uptimeMinutes++;
    if (relays_->GetFanState()) stats_.fanMinutes++;
    if (relays_->GetOzoneState()) stats_.ozoneMinutes++;
  }
}

/**
 * @brief Управление сохранением в EEPROM.
 * Сохраняет статистику каждые 30 минут для предотвращения потери данных.
 */
void Controller::HandleStorage() {
  unsigned long now = millis();
  if (now - last_eeprom_save_ >= 1800000UL) {
    last_eeprom_save_ = now;
    PersistentData data = {target_temp_, target_rh_, calib_, stats_, 0, 0};
    storage_.ScheduleSave(data, true);  // Принудительное немедленное сохранение
  }

  // Обслуживание очереди отложенной записи (таймеры внутри)
  storage_.Update();
}

/**
 * @brief Мониторинг работоспособности аппаратной части.
 */
void Controller::CheckSystemHealth() {
  // Проверка состояния шины I2C (датчики, экран, RTC)
  if (sensors_->IsI2cFailing()) {
#ifdef DEBUG
    Serial.println(F("I2C Fail detected. Recovering..."));
#endif
    sensors_->Recover();     // Попытка программного восстановления шины
    rtc_->Init();            // Переинициализация часов реального времени
    if (ui_) ui_->Reinit();  // Переинициализация LCD дисплея
  }

  // Постоянная проверка критических климатических параметров
  CheckCriticalErrors();

  // При обнаружении критической ошибки — переход в защищенный режим ERROR_STATE
  if (current_error_ != ErrorCode::kNone &&
      current_state_ != SystemState::kErrorState) {
    ChangeState(SystemState::kErrorState);
  }
}

/**
 * @brief Сброс флага присутствия пользователя по таймауту.
 */
void Controller::UpdateUserPresence() {
  if (is_user_present_ &&
      (millis() - last_user_activity_time_ >= kUserPresenceTimeout)) {
    is_user_present_ = false;
#ifdef DEBUG
    Serial.println(F("User Presence: Timed out"));
#endif
  }
}

/**
 * @brief Основная логика конечного автомата (FSM).
 * Обрабатывает автоматический климат, фазы озонирования и ручные режимы.
 */
void Controller::ProcessStateMachine() {
  unsigned long now = millis();
  switch (current_state_) {
    case SystemState::kAutoClimate:
      HandleAutoClimate();

      // Проверка расписания озонирования (через RTC)
      if (rtc_->IsOzoneTimeScheduled()) {
        ozone_retry_count_ = 0;
        ChangeState(SystemState::kOzoneStart);
      }
      // Повторная попытка после временной блокировки (через 30 мин)
      else if (retry_ozone_timer_ != 0 &&
               (now - retry_ozone_timer_ >= kOzoneRetryShort)) {
        retry_ozone_timer_ = 0;
        ChangeState(SystemState::kOzoneStart);
      }
      break;

    case SystemState::kOzoneStart: {
      // Проверка условий безопасности перед пуском газа
      SensorData out = sensors_->GetOutside();
      bool is_frost_outside = (out.temp <= kOutTempFrostLimit);
      bool is_ozone_inhibited = (is_frost_outside || is_user_present_);

      if (is_ozone_inhibited) {
        ozone_retry_count_++;
        if (ozone_retry_count_ >= kOzoneMaxRetriesPerDay) {
          retry_ozone_timer_ = 0;
          ozone_retry_count_ = 0;
#ifdef DEBUG
          Serial.println(F("Ozone: max retries reached, skipping today."));
#endif
          ChangeState(SystemState::kAutoClimate);
        } else {
#ifdef DEBUG
          Serial.println(F("Ozone Inhibited: Wait 30m"));
#endif
          retry_ozone_timer_ = now;
          ChangeState(SystemState::kAutoClimate);
        }
      } else {
        // Условия в норме - запуск генерации озона
        (void)relays_->SetFan(false, true); // Вентилятор ВЫКЛ (принудительно)
        relays_->SetOzone(true);      // Озонатор ВКЛ
        state_timer_ = now;
        ChangeState(SystemState::kOzoneActive);
      }
      break;
    }

    case SystemState::kOzoneActive:
      if (is_user_present_) {
        ChangeState(SystemState::kOzoneAbort);
        break;
      }
      // Фаза активной работы генератора (15 мин)
      if (now - state_timer_ >= kOzoneWorkTime) {
        relays_->SetOzone(false);
        state_timer_ = now;
        ChangeState(SystemState::kOzoneHold);
      }
      break;

    case SystemState::kOzoneHold:
      if (is_user_present_) {
        ChangeState(SystemState::kOzoneAbort);
        break;
      }
      // Фаза экспозиции (озон должен подействовать) (2 часа)
      if (now - state_timer_ >= kOzoneHoldTime) {
        state_timer_ = now;
        ChangeState(SystemState::kOzoneVent);
      }
      break;

    case SystemState::kOzoneVent: {
      // Фаза проветривания после озонирования (15 мин)
      SensorData out = sensors_->GetOutside();
      bool is_frost_outside = (out.temp <= kOutTempFrostLimit);

      // Проветривание разрешено только если на улице не мороз
      if (!is_frost_outside) {
        (void)relays_->SetFan(true);
        if (now - state_timer_ >= kOzoneVentTime) {
          (void)relays_->SetFan(false);
          ChangeState(SystemState::kAutoClimate);
        }
      } else {
        // На улице похолодало — прекращаем продувку во избежание заморозки
        (void)relays_->SetFan(false);
        ChangeState(SystemState::kAutoClimate);
      }
      break;
    }

    case SystemState::kOzoneAbort:
      // Экстренная остановка озонирования (например, пришел человек)
      relays_->SetOzone(false);
      (void)relays_->SetFan(false, true);
      ChangeState(SystemState::kAutoClimate);
      break;

    case SystemState::kManualFan:
      // Ручной запуск вентиляции по таймеру
      if (now - state_timer_ >= (manual_timer_ * 60000UL)) {
        (void)relays_->SetFan(false);
        ChangeState(SystemState::kAutoClimate);
      }
      break;

    case SystemState::kManualOzone:
      // Ручной запуск озонирования по таймеру
      if (now - state_timer_ >= (manual_timer_ * 60000UL)) {
        relays_->SetOzone(false);
        (void)relays_->SetFan(true);
        state_timer_ = now;
        ChangeState(SystemState::kOzoneVent);
      }
      break;

    case SystemState::kErrorState:
      // Режим ошибки — полная блокировка оборудования
      (void)relays_->SetFan(false, true);
      relays_->SetOzone(false);
      break;

    default:
      break;
  }
}

/**
 * @brief Алгоритм автоматического климат-контроля.
 * Использует 6-ступенчатую систему проверок для принятия решения о вентиляции.
 * Реализует двухпороговый гистерезис для предотвращения дребезга реле.
 */
void Controller::HandleAutoClimate() {
  SensorData in = sensors_->GetInside();
  SensorData out = sensors_->GetOutside();

  // Если данные датчиков невалидны — выключаем вентилятор и выходим
  if (!in.valid || !out.valid) {
    (void)relays_->SetFan(false);
    return;
  }

  // 1. Физические условия (Гистерезис)
  bool is_fan_on = relays_->GetFanState();
  // Пороги переключаются в зависимости от текущего состояния вентилятора
  bool is_too_hot = is_fan_on ? (in.temp > (target_temp_ - kHysteresisTemp))
                              : (in.temp > (target_temp_ + kHysteresisTemp));
  bool is_too_humid = is_fan_on ? (in.rh > (target_rh_ - kHysteresisRh))
                                : (in.rh > (target_rh_ + kHysteresisRh));

  // 2. Эффективность по влажности: уличный воздух должен быть суше
  bool ventilation_is_effective = (out.ah + kMarginAh) < in.ah;

  // 3. Защита от конденсата: точка росы улицы должна быть ниже температуры внутри
  bool condensation_is_safe = (out.dewpoint + kMarginCondSafety) < in.temp;

  // 4. Защита от промерзания: блокировка при морозе на улице
  bool freeze_safe = (out.temp > kOutTempFrostLimit);

  // 5. Эффективность охлаждения: не греть, если нужно охлаждать
  bool cooling_is_possible = !is_too_hot || (out.temp < in.temp);

  // 6. Минимальная граница Т внутри: не выстужать подвал ниже критического уровня
  bool temp_allows_vent = (in.temp > kVentTempMin);

  // Итоговое решение
  bool ventilation_needed = (is_too_hot || is_too_humid);

  // Вентилятор включается только при соблюдении ВСЕХ условий эффективности и безопасности
  if (ventilation_needed && ventilation_is_effective && condensation_is_safe &&
      freeze_safe && cooling_is_possible && temp_allows_vent) {
    bool res = relays_->SetFan(true);
    (void)res;
#ifdef DEBUG
    if (!res && !relays_->GetFanState()) {
      Serial.println(F("AutoClimate: FAN ON blocked by debounce"));
    }
#endif
  } else {
    bool res = relays_->SetFan(false);  // Выключение (с учетом защиты от частого переключения в RelayManager)
    (void)res;
#ifdef DEBUG
    if (!res && relays_->GetFanState()) {
      Serial.println(F("AutoClimate: FAN OFF blocked by debounce"));
    }
#endif
  }
}

/**
 * @brief Проверка аварийных ситуаций.
 */
void Controller::CheckCriticalErrors() {
  // 1. Ошибки оборудования (SensorManager)
  ErrorCode sErr = sensors_->CheckErrors();
  if (sErr != ErrorCode::kNone) {
    current_error_ = sErr;
    return;
  }

  // 2. Ошибки часов реального времени (RTC)
  ErrorCode tErr = rtc_->CheckErrors();
  if (tErr != ErrorCode::kNone) {
    current_error_ = tErr;
    return;
  }

  // 3. Проверка критического переохлаждения или риска затопления/конденсата
  SensorData in = sensors_->GetInside();
  if (in.temp <= kTempCriticalMin) {
    current_error_ = ErrorCode::kTempTooLow;
  } else if (in.dewpoint >= (in.temp - kCondensationErrDiff)) {
    current_error_ = ErrorCode::kCondensationRisk;
  } else {
    // Ошибок нет — сброс кода ошибки (позволяет системе восстанавливаться самостоятельно)
    current_error_ = ErrorCode::kNone;
  }
}

/**
 * @brief Реализация переключения состояний FSM.
 */
void Controller::ChangeState(SystemState new_state) {
  // Действия при переходе в состояние ОШИБКА
  if (new_state == SystemState::kErrorState) {
    (void)relays_->SetFan(false, true); // Немедленное выключение
    relays_->SetOzone(false);
  }

#ifdef DEBUG
  Serial.print(F("FSM: "));
  Serial.print((int)current_state_);
  Serial.print(F(" -> "));
  Serial.println((int)new_state);
#endif
  current_state_ = new_state;
}

/**
 * @brief Команда сброса ошибки пользователем.
 */
void Controller::ResetError() {
  current_error_ = ErrorCode::kNone;
  ChangeState(SystemState::kIdle);
  Init();  // Повторная попытка запуска
}

/**
 * @brief Ручной запуск вентиляции.
 */
void Controller::StartManualFan(uint16_t minutes) {
  if (current_state_ == SystemState::kErrorState) return;
  manual_timer_ = minutes;
  state_timer_ = millis();
  (void)relays_->SetFan(true, true);
  ChangeState(SystemState::kManualFan);
}

/**
 * @brief Ручной запуск озонирования.
 */
void Controller::StartManualOzone(uint16_t minutes) {
  if (current_state_ == SystemState::kErrorState) return;

  // Проверка условий безопасности (люди или мороз)
  SensorData out = sensors_->GetOutside();
  if (is_user_present_ || out.temp <= kOutTempFrostLimit) return;

  manual_timer_ = minutes;
  state_timer_ = millis();
  relays_->SetOzone(true);
  ChangeState(SystemState::kManualOzone);
}

/**
 * @brief Регистрация активности пользователя.
 * Используется для предотвращения озонирования при нахождении людей в подвале.
 */
void Controller::NotifyUserActivity() {
  is_user_present_ = true;
  last_user_activity_time_ = millis();
}

/**
 * @brief Изменение целевой температуры с записью в EEPROM.
 */
void Controller::SetTargetTemp(float t) {
  target_temp_ = t;
  PersistentData data = {target_temp_, target_rh_, calib_, stats_, 0, 0};
  storage_.ScheduleSave(data);
}

/**
 * @brief Изменение целевой влажности с записью в EEPROM.
 */
void Controller::SetTargetRh(float h) {
  target_rh_ = h;
  PersistentData data = {target_temp_, target_rh_, calib_, stats_, 0, 0};
  storage_.ScheduleSave(data);
}

/**
 * @brief Применение калибровки датчиков.
 */
void Controller::SetCalibration(const CalibrationData& data) {
  calib_ = data;
  sensors_->SetCalibration(calib_);
  PersistentData d = {target_temp_, target_rh_, calib_, stats_, 0, 0};
  storage_.ScheduleSave(d);
}

/**
 * @brief Сброс статистики наработки.
 */
void Controller::ResetStats() {
  stats_ = {0, 0, 0};
  PersistentData data = {target_temp_, target_rh_, calib_, stats_, 0, 0};
  storage_.ScheduleSave(data);
}
