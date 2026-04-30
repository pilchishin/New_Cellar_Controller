#include "Controller.h"
#include "DisplayUI.h"

Controller::Controller(SensorManager* s, RelayManager* r, TimeManager* t)
    : sensors_(s), relays_(r), rtc_(t), ui_(nullptr) {
  current_state_ = SystemState::kIdle;
  current_error_ = ErrorCode::kNone;

  // Загрузка данных из EEPROM
  PersistentData data;
  storage_.Load(data);

  target_temp_ = data.targetTemp;
  target_rh_ = data.targetRh;
  calib_ = data.calibration;
  stats_ = data.stats;

  sensors_->SetCalibration(calib_);

  last_stats_update_ = millis();
  last_eeprom_save_ = millis();
  is_user_present_ = false;
  last_user_activity_time_ = 0;
  state_timer_ = 0;
  manual_timer_ = 0;
  retry_ozone_timer_ = 0;
}

void Controller::Init() { ChangeState(SystemState::kAutoClimate); }

void Controller::Tick() {
  UpdateStatistics();
  HandleStorage();
  CheckSystemHealth();
  UpdateUserPresence();
  ProcessStateMachine();
}

void Controller::UpdateStatistics() {
  unsigned long now = millis();
  // Обновление статистики (раз в минуту)
  if (now - last_stats_update_ >= 60000UL) {
    last_stats_update_ = now;
    stats_.uptimeMinutes++;
    if (relays_->GetFanState()) stats_.fanMinutes++;
    if (relays_->GetOzoneState()) stats_.ozoneMinutes++;
  }
}

void Controller::HandleStorage() {
  unsigned long now = millis();
  // Сохранение статистики в EEPROM (раз в 30 минут)
  if (now - last_eeprom_save_ >= 1800000UL) {
    last_eeprom_save_ = now;
    PersistentData data = {target_temp_, target_rh_, calib_, stats_, 0};
    storage_.ScheduleSave(data, true);  // Принудительное (немедленное) сохранение
  }

  // Обслуживание очереди записи (проверка таймеров внутри)
  storage_.Update();
}

void Controller::CheckSystemHealth() {
  // Проверка зависания шины I2C
  if (sensors_->IsI2cFailing()) {
#ifdef DEBUG
    Serial.println(F("I2C Fail detected. Recovering..."));
#endif
    sensors_->Recover();  // Сброс шины + переинициализация датчиков
    rtc_->Init();         // Переинициализация RTC
    if (ui_) ui_->Reinit();  // Переинициализация LCD
  }

  // Постоянная проверка критических ошибок
  CheckCriticalErrors();

  // Если есть критическая ошибка — принудительный переход в ERROR_STATE
  if (current_error_ != ErrorCode::kNone &&
      current_state_ != SystemState::kErrorState) {
    ChangeState(SystemState::kErrorState);
  }
}

void Controller::UpdateUserPresence() {
  if (is_user_present_ &&
      (millis() - last_user_activity_time_ >= kUserPresenceTimeout)) {
    is_user_present_ = false;
#ifdef DEBUG
    Serial.println(F("User Presence: Timed out"));
#endif
  }
}

void Controller::ProcessStateMachine() {
  unsigned long now = millis();
  // Обработка состояний (FSM)
  switch (current_state_) {
    case SystemState::kAutoClimate:
      HandleAutoClimate();

      // Переход к озонированию по расписанию
      if (rtc_->IsOzoneTimeScheduled()) {
        ChangeState(SystemState::kOzoneStart);
      }
      // Повторная попытка озонирования по таймеру (после блокировки)
      else if (retry_ozone_timer_ != 0 &&
               (now - retry_ozone_timer_ >= kOzoneRetryShort)) {
        retry_ozone_timer_ = 0;
        ChangeState(SystemState::kOzoneStart);
      }
      break;

    case SystemState::kOzoneStart: {
      // Проверка условий блокировки (присутствие людей или мороз)
      SensorData out = sensors_->GetOutside();
      bool is_frost_outside = (out.temp < 0.0f);

      // Проверяем внутренний флаг присутствия вместо прямого обращения к UI
      bool is_ozone_inhibited = (is_frost_outside || is_user_present_);

      if (is_ozone_inhibited) {
#ifdef DEBUG
        Serial.println(F("Ozone Inhibited: Wait 30m"));
#endif
        retry_ozone_timer_ = now;
        ChangeState(SystemState::kAutoClimate);
      } else {
        // Условия в норме - запуск фазы генерации
        relays_->SetFan(false, true);
        relays_->SetOzone(true);
        state_timer_ = now;
        ChangeState(SystemState::kOzoneActive);
      }
      break;
    }

    case SystemState::kOzoneActive:
      if (now - state_timer_ >= kOzoneWorkTime) {
        relays_->SetOzone(false);
        state_timer_ = now;
        ChangeState(SystemState::kOzoneHold);
      }
      break;

    case SystemState::kOzoneHold:
      if (now - state_timer_ >= kOzoneHoldTime) {
        state_timer_ = now;
        ChangeState(SystemState::kOzoneVent);
      }
      break;

    case SystemState::kOzoneVent: {
      SensorData out = sensors_->GetOutside();
      bool is_frost_outside = (out.temp <= 0.0f);

      // Проветривание разрешено только при плюсовой температуре
      if (!is_frost_outside) {
        relays_->SetFan(true);
        if (now - state_timer_ >= kOzoneVentTime) {
          relays_->SetFan(false);
          ChangeState(SystemState::kAutoClimate);
        }
      } else {
        // Прекращаем проветривание, если на улице похолодало
        relays_->SetFan(false);
        ChangeState(SystemState::kAutoClimate);
      }
      break;
    }

    case SystemState::kOzoneAbort:
      relays_->SetFan(false, true);
      relays_->SetOzone(false);
      ChangeState(SystemState::kAutoClimate);
      break;

    case SystemState::kManualFan:
      if (now - state_timer_ >= (manual_timer_ * 60000UL)) {
        relays_->SetFan(false);
        ChangeState(SystemState::kAutoClimate);
      }
      break;

    case SystemState::kManualOzone:
      if (now - state_timer_ >= (manual_timer_ * 60000UL)) {
        relays_->SetOzone(false);
        ChangeState(SystemState::kAutoClimate);
      }
      break;

    case SystemState::kErrorState:
      relays_->SetFan(false, true);  // Выключить всё немедленно
      relays_->SetOzone(false);
      break;

    default:
      break;
  }
}

/**
 * @brief Алгоритм автоматического климат-контроля.
 * Решает, нужно ли включать вентиляцию, основываясь на разнице
 * абсолютной влажности (AH), температуре и риске конденсата.
 */
void Controller::HandleAutoClimate() {
  SensorData in = sensors_->GetInside();
  SensorData out = sensors_->GetOutside();

  if (!in.valid || !out.valid) {
    relays_->SetFan(false);
    return;
  }

  // 1. Физические условия для активации вентиляции
  bool is_fan_on = relays_->GetFanState();
  bool is_too_hot = is_fan_on ? (in.temp > (target_temp_ - kHisteresisTempOff))
                              : (in.temp > (target_temp_ + kHysteresisTemp));
  bool is_too_humid = is_fan_on ? (in.rh > (target_rh_ - kHysteresisRh))
                                : (in.rh > (target_rh_ + kHysteresisRh));

  // 2. Эффективность: воздух снаружи должен содержать меньше влаги
  bool ventilation_is_effective = (out.ah + kMarginAh) < in.ah;

  // 3. Безопасность: температура поверхностей должна быть выше точки росы
  bool condensation_is_safe = (out.dewpoint + kMarginCondSafety) < in.temp;

  // 4. Защита от промерзания
  bool freeze_safe = (out.temp > kOutTempFrostLimit);

  // 5. Проверка возможности охлаждения
  bool cooling_is_possible = !is_too_hot || (out.temp < in.temp);

  // Итоговая логика принятия решения
  bool ventilation_needed = (is_too_hot || is_too_humid);

  if (ventilation_needed && ventilation_is_effective && condensation_is_safe && freeze_safe && cooling_is_possible) {
    relays_->SetFan(true);
  } else {
    relays_->SetFan(false);  // Выключение (с учетом защиты в RelayManager)
  }
}


void Controller::CheckCriticalErrors() {
  // 1. Ошибки от SensorManager (Hardware + Mismatch)
  ErrorCode sErr = sensors_->CheckErrors();
  if (sErr != ErrorCode::kNone) {
    current_error_ = sErr;
    return;
  }

  // 2. Ошибки от TimeManager
  ErrorCode tErr = rtc_->CheckErrors();
  if (tErr != ErrorCode::kNone) {
    current_error_ = tErr;
    return;
  }

  // 3. Проверка параметров климата
  SensorData in = sensors_->GetInside();
  if (in.temp <= kTempCriticalMin) {
    current_error_ = ErrorCode::kTempTooLow;
  } else if (in.dewpoint >= (in.temp - kCondensationErrDiff)) {
    current_error_ = ErrorCode::kCondensationRisk;
  } else {
    current_error_ = ErrorCode::kNone;
  }
}

void Controller::ChangeState(SystemState new_state) {
  if (new_state == SystemState::kErrorState) {
    relays_->SetFan(false, true);
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

void Controller::ResetError() {
  // Ручной сброс ошибки возможен только если физическая причина устранена
  current_error_ = ErrorCode::kNone;
  ChangeState(SystemState::kIdle);
  Init();  // Пробуем запуститься снова
}

void Controller::StartManualFan(uint16_t minutes) {
  if (current_state_ == SystemState::kErrorState) return;
  manual_timer_ = minutes;
  state_timer_ = millis();
  relays_->SetFan(true, true);
  ChangeState(SystemState::kManualFan);
}

void Controller::StartManualOzone(uint16_t minutes) {
  if (current_state_ == SystemState::kErrorState) return;
  manual_timer_ = minutes;
  state_timer_ = millis();
  relays_->SetOzone(true);
  ChangeState(SystemState::kManualOzone);
}

void Controller::NotifyUserActivity() {
  is_user_present_ = true;
  last_user_activity_time_ = millis();
}

void Controller::SetTargetTemp(float t) {
  target_temp_ = t;
  PersistentData data = {target_temp_, target_rh_, calib_, stats_, 0};
  storage_.ScheduleSave(data);
}

void Controller::SetTargetRh(float h) {
  target_rh_ = h;
  PersistentData data = {target_temp_, target_rh_, calib_, stats_, 0};
  storage_.ScheduleSave(data);
}

void Controller::SetCalibration(const CalibrationData& data) {
  calib_ = data;
  sensors_->SetCalibration(calib_);
  PersistentData d = {target_temp_, target_rh_, calib_, stats_, 0};
  storage_.ScheduleSave(d);
}

void Controller::ResetStats() {
  stats_ = {0, 0, 0};
  PersistentData data = {target_temp_, target_rh_, calib_, stats_, 0};
  storage_.ScheduleSave(data);
}
