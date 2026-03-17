#ifndef UI_MODEL_H
#define UI_MODEL_H

#include "Types.h"

// Предварительные объявления классов
class Controller;
class SensorManager;

/**
 * @class UIModel
 * @brief Слой данных для отображения, отделяющий DisplayUI от логики Controller.
 * Хранит локальную копию всех данных, необходимых для отрисовки интерфейса.
 */
class UIModel {
 public:
  UIModel();

  /**
   * @brief Синхронизация данных модели с актуальными значениями из системы.
   * @param controller Ссылка на контроллер для получения установок и состояния.
   * @param sensors Ссылка на менеджер датчиков для получения показаний.
   */
  void Sync(Controller* controller, SensorManager* sensors);

  // Геттеры данных (публичные поля для простоты доступа из DisplayUI)
  SensorData inside;
  SensorData outside;
  SystemState state;
  ErrorCode error;

  float target_temp;
  float target_rh;

  CalibrationData calib;
  SystemStatistics stats;

  bool fan_on;
  bool ozone_on;

  // Дополнительные флаги для UI
  bool is_manual_mode;
  bool is_auto_mode;
};

#endif
