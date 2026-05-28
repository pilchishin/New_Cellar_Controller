#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <Arduino.h>
#include "Types.h"
#include "config.h"
#include "SensorManager.h"
#include "RelayManager.h"
#include "TimeManager.h"
#include "AppEEPROM.h"

// Предварительное объявление для устранения циклической зависимости
class DisplayUI; 

/**
 * @class Controller
 * @brief Центральный модуль управления системой (Координатор).
 * Реализует логику конечного автомата (FSM), управляет климатом,
 * озонированием, хранением настроек и взаимодействием между периферийными модулями.
 */
class Controller {
 private:
  SystemState current_state_; ///< Текущее состояние системы
  ErrorCode current_error_;   ///< Код активной ошибки (ErrorCode::kNone если все ОК)

  // Уставки климата (загружаются из EEPROM)
  float target_temp_;         ///< Целевая температура (°C)
  float target_rh_;           ///< Целевая влажность (%)

  // Калибровочные данные
  CalibrationData calib_;     ///< Смещения датчиков

  // Статистика работы
  SystemStatistics stats_;    ///< Время работы компонентов
  unsigned long last_stats_update_; ///< Таймер для ежеминутного обновления статистики
  unsigned long last_eeprom_save_;  ///< Таймер для периодического сохранения в EEPROM

  // Модуль энергонезависимой памяти
  AppEEPROM storage_;

  // Указатели на другие модули системы
  SensorManager* sensors_;
  RelayManager* relays_;
  TimeManager* rtc_;
  DisplayUI* ui_;

  // Переменные внутреннего состояния логики
  unsigned long state_timer_;        ///< Универсальный таймер для фаз озонирования и ручных режимов
  uint16_t manual_timer_;            ///< Длительность ручного режима в минутах
  unsigned long retry_ozone_timer_;  ///< Таймер для отложенного перезапуска озонатора при запрете (мороз/люди)

  // Логика обнаружения присутствия пользователя (для безопасности озонирования)
  bool is_user_present_;             ///< Флаг недавней активности пользователя
  unsigned long last_user_activity_time_; ///< Метка времени последнего действия пользователя

  /**
   * @brief Реализация алгоритма автоматического поддержания климата.
   * Вычисляет 6 условий (необходимость, эффективность, точка росы, мороз, охлаждение, лимит T)
   * и принимает решение о включении вентилятора.
   */
  void HandleAutoClimate();

  /**
   * @brief Постоянный мониторинг критических параметров системы.
   * Проверяет аппаратные сбои и выход климата за аварийные границы.
   */
  void CheckCriticalErrors();

  /**
   * @brief Безопасный переход между состояниями FSM.
   * Выполняет entry-actions (например, выключение реле при переходе в ошибку).
   * @param new_state Целевое состояние.
   */
  void ChangeState(SystemState new_state);

  // Вспомогательные методы декомпозиции Tick()
  void UpdateStatistics();     ///< Обновление счетчиков моточасов
  void HandleStorage();        ///< Управление жизненным циклом EEPROM
  void CheckSystemHealth();    ///< Контроль шины I2C и аппаратного здоровья
  void UpdateUserPresence();   ///< Обновление флага присутствия человека
  void ProcessStateMachine();  ///< Главный диспетчер состояний (FSM switch)

 public:
  /**
   * @brief Конструктор контроллера.
   * @param s Менеджер датчиков.
   * @param r Менеджер реле.
   * @param t Менеджер времени (RTC).
   */
  Controller(SensorManager* s, RelayManager* r, TimeManager* t);

  /**
   * @brief Регистрация модуля интерфейса.
   * Вызывается отдельно после инициализации UI.
   */
  void SetUI(DisplayUI* u) { ui_ = u; }

  /**
   * @brief Начальная инициализация логики (переход в AutoClimate).
   */
  void Init();

  /**
   * @brief Главный итерационный метод. Должен вызываться в каждом цикле loop().
   */
  void Tick();

  // Методы управления, вызываемые из UI/Меню
  void ResetError();                    ///< Сброс аварийного состояния
  void StartManualFan(uint16_t minutes);  ///< Запуск вентилятора на время
  void StartManualOzone(uint16_t minutes); ///< Запуск озонатора на время
  void NotifyUserActivity();            ///< Регистрация нажатия кнопок пользователем

  // Геттеры состояния для отображения в UI
  SystemState GetState() const { return current_state_; }
  ErrorCode GetError() const { return current_error_; }
  RelayManager* GetRelayManager() const { return relays_; }

  // Работа с настройками (с автоматическим сохранением в EEPROM)
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
