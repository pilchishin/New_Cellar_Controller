#include "Controller.h"
#include "DisplayUI.h"

Controller::Controller(SensorManager* s, RelayManager* r, TimeManager* t)
    : sensors_(s), relays_(r), rtc_(t), ui_(nullptr) {
  current_state_ = SystemState::kIdle;
  current_error_ = ErrorCode::kNone;

  // Загрузка данных из EEPROM
  PersistentData data;
  storage_.load(data);

  target_temp_ = data.targetTemp;
  target_rh_ = data.targetRh;
  calib_ = data.calibration;
  stats_ = data.stats;

  sensors_->SetCalibration(calib_);

  last_stats_update_ = millis();
  last_eeprom_save_ = millis();
  needs_persistent_save_ = false;
  last_persistent_change_time_ = 0;
  ozone_inhibited_today_ = false;
  state_timer_ = 0;
  manual_timer_ = 0;
  retry_ozone_timer_ = 0;
}

void Controller::Init() { ChangeState(SystemState::kAutoClimate); }

void Controller::Tick() {
  UpdateStatistics();
  HandleStorage();
  CheckSystemHealth();
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
  // 0.1 Сохранение статистики в EEPROM (раз в 30 минут)
  if (now - last_eeprom_save_ >= 1800000UL) {
    last_eeprom_save_ = now;
    needs_persistent_save_ = true;
    last_persistent_change_time_ =
        now - kDeferredSaveDelay;  // Принудительное сохранение без задержки
  }

  // 0.2 Отложенное сохранение в EEPROM
  if (needs_persistent_save_ &&
      (now - last_persistent_change_time_ >= kDeferredSaveDelay)) {
    needs_persistent_save_ = false;
    PersistentData data = {target_temp_, target_rh_, calib_, stats_, 0};
    storage_.save(data);
  }
}

void Controller::CheckSystemHealth() {
  // Проверка зависания шины I2C
  if (sensors_->IsI2cFailing()) {
#ifdef DEBUG
    Serial.println(F("I2C Fail detected. Recovering..."));
#endif
    sensors_->Recover();  // Сброс шины + переинициализация датчиков
    rtc_->Init();         // Переинициализация RTC
    ui_->Reinit();        // Переинициализация LCD
  }

  // Постоянная проверка критических ошибок
  CheckCriticalErrors();

  // Если есть критическая ошибка — принудительный переход в ERROR_STATE
  if (current_error_ != ErrorCode::kNone &&
      current_state_ != SystemState::kErrorState) {
    ChangeState(SystemState::kErrorState);
  }
}

void Controller::ProcessStateMachine() {
  // Обработка состояний (FSM)
  switch (current_state_) {
    case SystemState::kAutoClimate:
      HandleAutoClimate();

      // Проверка запуска озонирования по расписанию
      if (rtc_->IsOzoneTimeScheduled()) {
        ChangeState(SystemState::kOzoneStart);
      }
      // Проверка повторной попытки через 30 минут (если был запрет)
      else if (retry_ozone_timer_ != 0 &&
               (millis() - retry_ozone_timer_ >= kOzoneRetryShort)) {
        retry_ozone_timer_ = 0;  // Сбрасываем таймер
        ChangeState(SystemState::kOzoneStart);
      }
      break;

    case SystemState::kOzoneStart:
      HandleOzoneCycle();  // Проверка условий запуска (T_out и подсветка)
      break;

    case SystemState::kOzoneActive:
    case SystemState::kOzoneHold:
    case SystemState::kOzoneVent:
      HandleOzoneCycle();  // Логика фаз
      break;

    case SystemState::kManualFan:
    case SystemState::kManualOzone:
      HandleManualModes();
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

  // Логика активации: если превышен порог температуры или влажности
  bool needs_action = (in.temp > (target_temp_ + kHysteresisTemp)) ||
                      (in.rh > (target_rh_ + kHysteresisRh));

  // Проверка, что на улице воздух действительно суше, чем внутри
  bool air_is_better = (out.ah + kMarginAh) < in.ah;

  // Проверка безопасности: не допустить охлаждения поверхностей ниже точки росы
  bool condensation_safe = (in.dewpoint + kMarginCondSafety) < in.temp;

  // Итоговое решение по вентилятору
  if (needs_action && air_is_better && condensation_safe) {
    relays_->SetFan(true);
  } else {
    relays_->SetFan(false);  // Включается гистерезис и защита двигателя в RelayManager
  }
}

/**
 * @brief Управление многофазным циклом озонирования.
 * Фазы: Ожидание -> Озонирование (15м) -> Экспозиция (2ч) -> Проветривание (15м).
 */
void Controller::HandleOzoneCycle() {
  SensorData out = sensors_->GetOutside();

  // Проверка условий блокировки (только в момент старта)
  if (current_state_ == SystemState::kOzoneStart) {
    bool temp_inhibited = (out.temp < 0.0f);
    bool ui_inhibited = (ui_ != nullptr && ui_->isBacklightOn());

    if (temp_inhibited || ui_inhibited) {
#ifdef DEBUG
      Serial.println(F("Ozone Inhibited: Wait 30m"));
#endif
      retry_ozone_timer_ = millis();
      ChangeState(SystemState::kAutoClimate);
      return;
    }

    // Если всё ок — включаем озон
    relays_->SetFan(false, true);
    relays_->SetOzone(true);
    state_timer_ = millis();
    ChangeState(SystemState::kOzoneActive);
  }

  // Фаза 1: Активная работа озонатора (генерация озона)
  if (current_state_ == SystemState::kOzoneActive) {
    if (millis() - state_timer_ >= kOzoneWorkTime) {
      relays_->SetOzone(false);
      state_timer_ = millis();
      ChangeState(SystemState::kOzoneHold);
    }
  }

  // Фаза 2: Пауза (ожидание распада озона)
  if (current_state_ == SystemState::kOzoneHold) {
    if (millis() - state_timer_ >= kOzoneHoldTime) {
      state_timer_ = millis();
      ChangeState(SystemState::kOzoneVent);
    }
  }

  // Фаза 3: Принудительное проветривание после обработки
  if (current_state_ == SystemState::kOzoneVent) {
    // Проветривание разрешено только если на улице не мороз
    if (out.temp > 0.0f) {
      relays_->SetFan(true);
      if (millis() - state_timer_ >= kOzoneVentTime) {
        relays_->SetFan(false);
        ChangeState(SystemState::kAutoClimate);
      }
    } else {
      // Если мороз — принудительно выходим
      relays_->SetFan(false);
      ChangeState(SystemState::kAutoClimate);
    }
  }
}

void Controller::HandleManualModes() {
  if (millis() - state_timer_ >= (manual_timer_ * 60000UL)) {
    relays_->SetFan(false);
    relays_->SetOzone(false);
    ChangeState(SystemState::kAutoClimate);
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
  }
}

void Controller::ChangeState(SystemState new_state) {
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

void Controller::SetTargetTemp(float t) {
  target_temp_ = t;
  needs_persistent_save_ = true;
  last_persistent_change_time_ = millis();
}

void Controller::SetTargetRh(float h) {
  target_rh_ = h;
  needs_persistent_save_ = true;
  last_persistent_change_time_ = millis();
}

void Controller::SetCalibration(const CalibrationData& data) {
  calib_ = data;
  sensors_->SetCalibration(calib_);
  needs_persistent_save_ = true;
  last_persistent_change_time_ = millis();
}

void Controller::ResetStats() {
  stats_ = {0, 0, 0};
  needs_persistent_save_ = true;
  last_persistent_change_time_ = millis();
}
