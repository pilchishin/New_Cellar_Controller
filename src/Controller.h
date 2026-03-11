#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include "Types.h"
#include "config.h"
#include "SensorManager.h"
#include "RelayManager.h"
#include "TimeManager.h"
#include "AppEEPROM.h"

// Предварительное объявление, чтобы избежать циклической зависимости
class DisplayUI; 

class Controller {
 private:
  SystemState current_state_;
  ErrorCode current_error_;

  // Уставки климата
  float target_temp_;
  float target_rh_;

  // Калибровочные данные
  CalibrationData calib_;

  // Статистика
  SystemStatistics stats_;
  unsigned long last_stats_update_;
  unsigned long last_eeprom_save_;

  // EEPROM
  AppEEPROM storage_;

  // Ссылки на модули
  SensorManager* sensors_;
  RelayManager* relays_;
  TimeManager* rtc_;
  DisplayUI* ui_;

  // Переменные логики
  unsigned long state_timer_;        // Timer for ozone phases and manual modes
  uint16_t manual_timer_;            // Manual mode duration in minutes
  unsigned long retry_ozone_timer_;  // Таймер для повтора при запрете (30 мин)
  bool ozone_inhibited_today_;       // Флаг, что сегодня попытка уже была

  // Абстракция присутствия пользователя
  bool is_user_present_;
  unsigned long last_user_activity_time_;
  const unsigned long kUserPresenceTimeout = 30000UL;

  // Внутренние методы обработки состояний
  void HandleAutoClimate();

  // Проверка критических условий
  void CheckCriticalErrors();

  // Переход между состояниями
  void ChangeState(SystemState new_state);

  // Методы декомпозиции Tick()
  void UpdateStatistics();     // Обновление счетчиков времени работы
  void HandleStorage();        // Управление планированием записи в EEPROM
  void CheckSystemHealth();    // Мониторинг датчиков и шины I2C
  void UpdateUserPresence();   // Управление флагом присутствия пользователя
  void ProcessStateMachine();  // Логика переключения состояний (FSM)

 public:
  Controller(SensorManager* s, RelayManager* r, TimeManager* t);

  void SetUI(DisplayUI* u) { ui_ = u; }  // Установка UI после его инициализации

  void Init();
  void Tick();  // Основной цикл логики

  // Управление из UI
  void ResetError();
  void StartManualFan(uint16_t minutes);
  void StartManualOzone(uint16_t minutes);
  void NotifyUserActivity();  // Сообщение от UI об активности (нажатие кнопок)

  // Геттеры для UI
  SystemState GetState() const { return current_state_; }
  ErrorCode GetError() const { return current_error_; }
  RelayManager* GetRelayManager() const { return relays_; }

  float GetTargetTemp() const { return target_temp_; }
  float GetTargetRh() const { return target_rh_; }
  void SetTargetTemp(float t);
  void SetTargetRh(float h);

  CalibrationData GetCalibration() const { return calib_; }
  void SetCalibration(const CalibrationData& data);

  SystemStatistics GetStats() const { return stats_; }
  void ResetStats();
};

#endif
