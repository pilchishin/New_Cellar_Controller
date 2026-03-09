#include "RelayManager.h"

RelayManager::RelayManager() {
  fan_state_ = false;
  ozone_state_ = false;

  // Инициализируем нулем. Это создаст полезный побочный эффект:
  // при включении питания контроллера вентилятор не сможет включиться первые 5 минут.
  // Это даст датчикам время на прогрев и стабилизацию показаний (особенно BME280).
  last_fan_change_time_ = 0;
}

void RelayManager::Init() {
    #ifdef DEBUG
    Serial.println(F("Init Relays..."));
    #endif

    // Настраиваем пины управления как выходы
    pinMode(PIN_RELAY_FAN, OUTPUT);
    pinMode(PIN_RELAY_OZONE, OUTPUT);

    // Устанавливаем начальное безопасное состояние — ВСЕ ВЫКЛЮЧЕНО.
    // ПРИМЕЧАНИЕ: Твердотельные реле (SSR) обычно включаются высоким уровнем (HIGH).
    // Если используете инверсные модули реле, замените LOW на HIGH здесь и в методах ниже.
    digitalWrite(PIN_RELAY_FAN, LOW);
    digitalWrite(PIN_RELAY_OZONE, LOW);
}

/**
 * @brief Управление вентилятором с защитой от частого переключения.
 * @param requestedState true - включить, false - выключить.
 * @param force Если true, игнорировать 5-минутный интервал (для аварий).
 */
void RelayManager::SetFan(bool requested_state, bool force) {
  // Если состояние не меняется, выходим для экономии ресурсов
  if (this->fan_state_ == requested_state) {
    return;
  }

  unsigned long current_time = millis();

  // Логика защиты двигателя (антидребезг):
  // Включаем или выключаем только если прошло 5 минут с последнего переключения.
  // Флаг force позволяет обойти это правило при аварийных ситуациях.
  if (force || (current_time - last_fan_change_time_ >= kFanDebounceDelay)) {
    // Обновляем текущее состояние и сбрасываем таймер
    this->fan_state_ = requested_state;
    this->last_fan_change_time_ = current_time;

    // Подаем сигнал на физический пин
    digitalWrite(PIN_RELAY_FAN, requested_state ? HIGH : LOW);

#ifdef DEBUG
    Serial.print(F("FAN Relay changed to: "));
    Serial.println(requested_state ? F("ON") : F("OFF"));
    if (force) Serial.println(F(" (FORCED)"));
#endif
  } else {
        // Если 5 минут не прошло, команда игнорируется аппаратно.
        // FSM из Controller.cpp будет продолжать присылать команду setFan(true/false) каждый цикл,
        // и как только 5 минут истекут, условие выполнится и реле переключится.
    }
}

void RelayManager::SetOzone(bool requested_state) {
  // Защита от лишних вызовов digitalWrite
  if (this->ozone_state_ == requested_state) {
    return;
  }

  // Озонатор переключается мгновенно по запросу (логика времени заложена в самом FSM)
  this->ozone_state_ = requested_state;
  digitalWrite(PIN_RELAY_OZONE, requested_state ? HIGH : LOW);

#ifdef DEBUG
  Serial.print(F("OZONE Relay changed to: "));
  Serial.println(requested_state ? F("ON") : F("OFF"));
#endif
}