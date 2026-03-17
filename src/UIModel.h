#ifndef UI_MODEL_H
#define UI_MODEL_H

#include "Types.h"

// Предварительные объявления классов
class Controller;
class SensorManager;

/**
 * @enum ValueID
 * @brief Идентификаторы числовых параметров для доступа через UIModel.
 */
enum class ValueID : uint8_t {
  kNone,
  kTargetTemp,
  kTargetHum,
  kCalibBmeTemp,
  kCalibBmeHum,
  kCalibHtuTemp,
  kCalibHtuHum,
  kCalibDsTemp
};

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

  /**
   * @brief Получение значения параметра по его ID.
   */
  float GetValue(ValueID id) const;

  /**
   * @brief Установка значения параметра (локально в модели).
   */
  void SetValue(ValueID id, float v);

  // Геттеры данных (публичные поля для простоты доступа из DisplayUI)
  SensorData inside;
  SensorData outside;
  SystemState state;
  ErrorCode error;

  float target_temp;
  float target_rh;

  CalibrationData calib;
  float calib_bme_t;
  float calib_bme_h;
  float calib_htu_t;
  float calib_htu_h;
  float calib_ds_t;

  SystemStatistics stats;

  bool fan_on;
  bool ozone_on;

  // Дополнительные флаги для UI
  bool is_manual_mode;
  bool is_auto_mode;
};

#endif
