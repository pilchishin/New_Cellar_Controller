/**
 * @file RelayManager.cpp
 * @brief Реализация управления реле с логикой защиты нагрузки.
 */

#include "RelayManager.h"

/**
 * @brief Конструктор RelayManager.
 *
 * Устанавливает начальные состояния в 'false' (выключено).
 * Инициализация 'last_fan_change_time_' нулем создает защитный интервал
 * при старте системы: вентилятор не сможет включиться первые 5 минут работы
 * контроллера, что дает датчикам время на прогрев и стабилизацию показаний.
 */
RelayManager::RelayManager() {
  fan_state_ = false;
  ozone_state_ = false;
  last_fan_change_time_ = 0;
}

/**
 * @brief Конфигурация пинов управления реле.
 *
 * Настраивает пины на выход и принудительно переводит их в состояние LOW.
 * ПРИМЕЧАНИЕ: Твердотельные реле (SSR) в данном проекте управляются высоким уровнем (HIGH).
 */
void RelayManager::Init() {
    #ifdef DEBUG
    Serial.println(F("Init Relays..."));
    #endif

    pinMode(PIN_RELAY_FAN, OUTPUT);
    pinMode(PIN_RELAY_OZONE, OUTPUT);

    // Начальное безопасное состояние — ВСЕ ВЫКЛЮЧЕНО.
    digitalWrite(PIN_RELAY_FAN, LOW);
    digitalWrite(PIN_RELAY_OZONE, LOW);
}

/**
 * @brief Переключение состояния вентилятора с проверкой таймера защиты.
 *
 * @param requested_state Целевое состояние.
 * @param force Если true, таймер защиты игнорируется (критические ошибки).
 */
void RelayManager::SetFan(bool requested_state, bool force) {
  // Если запрашиваемое состояние уже активно, ничего не делаем
  if (this->fan_state_ == requested_state) {
    return;
  }

  unsigned long current_time = millis();

  // Логика защиты двигателя:
  // Переключение разрешено только если прошло kFanDebounceDelay (5 минут)
  // с момента последнего изменения. Это предотвращает порчу двигателя при
  // пограничных значениях температуры/влажности.
  if (force || (current_time - last_fan_change_time_ >= kFanDebounceDelay)) {
    this->fan_state_ = requested_state;
    this->last_fan_change_time_ = current_time;

    // Управление физическим уровнем на пине
    digitalWrite(PIN_RELAY_FAN, requested_state ? HIGH : LOW);

#ifdef DEBUG
    Serial.print(F("FAN Relay changed to: "));
    Serial.println(requested_state ? F("ON") : F("OFF"));
    if (force) Serial.println(F(" (FORCED)"));
#endif
  }
  // Если интервал не прошел, команда игнорируется.
  // Контроллер продолжит вызывать этот метод в каждом цикле,
  // и реле сработает сразу, как только время защиты истечет.
}

/**
 * @brief Переключение состояния озонатора.
 *
 * Выполняется мгновенно при вызове, так как озонатор не требует
 * аппаратной защиты от частого переключения на уровне этого менеджера.
 *
 * @param requested_state Целевое состояние.
 */
void RelayManager::SetOzone(bool requested_state) {
  if (this->ozone_state_ == requested_state) {
    return;
  }

  this->ozone_state_ = requested_state;
  digitalWrite(PIN_RELAY_OZONE, requested_state ? HIGH : LOW);

#ifdef DEBUG
  Serial.print(F("OZONE Relay changed to: "));
  Serial.println(requested_state ? F("ON") : F("OFF"));
#endif
}
