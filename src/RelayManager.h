#ifndef RELAY_MANAGER_H
#define RELAY_MANAGER_H

#include <Arduino.h>
#include "Config.h" // Предполагается, что здесь заданы пины PIN_RELAY_FAN и PIN_RELAY_OZONE

class RelayManager {
 private:
  // Текущие логические состояния реле
  bool fan_state_;
  bool ozone_state_;

  // Временная метка последнего переключения вентилятора (для антидребезга)
  unsigned long last_fan_change_time_;

  // Константа: 5 минут в миллисекундах (5 * 60 * 1000)
  const unsigned long kFanDebounceDelay = 300000UL;

 public:
  // Конструктор
  RelayManager();

  // Инициализация пинов (настройка OUTPUT и начального состояния)
  void Init();

  /**
   * @brief Управление вентилятором с защитой от частого переключения.
   * @param requested_state true - включить, false - выключить.
   * @param force Если true, игнорировать 5-минутный интервал (для аварий).
   */
  void SetFan(bool requested_state, bool force = false);

  // Установка состояния озонатора.
  void SetOzone(bool requested_state);

  // Геттеры для отображения статусов на экране LCD (в меню Status)
  bool GetFanState() const { return fan_state_; }
  bool GetOzoneState() const { return ozone_state_; }
};

#endif