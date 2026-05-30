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

  // Getters
  SensorData GetInside() const { return inside_; }
  SensorData GetOutside() const { return outside_; }
  SystemState GetState() const { return state_; }
  ErrorCode GetError() const { return error_; }
  float GetTargetTemp() const { return target_temp_; }
  float GetTargetRh() const { return target_rh_; }
  CalibrationData GetCalibration() const { return calib_; }
  float GetCalibBmeT() const { return calib_bme_t_; }
  float GetCalibBmeH() const { return calib_bme_h_; }
  float GetCalibHtuT() const { return calib_htu_t_; }
  float GetCalibHtuH() const { return calib_htu_h_; }
  float GetCalibDsT() const { return calib_ds_t_; }
  SystemStatistics GetStats() const { return stats_; }
  bool IsFanOn() const { return fan_on_; }
  bool IsOzoneOn() const { return ozone_on_; }
  bool IsManualMode() const { return is_manual_mode_; }
  bool IsAutoMode() const { return is_auto_mode_; }

  // Setters
  void SetInside(const SensorData& v) { inside_ = v; }
  void SetOutside(const SensorData& v) { outside_ = v; }
  void SetState(SystemState v) { state_ = v; }
  void SetError(ErrorCode v) { error_ = v; }
  void SetTargetTemp(float v) { target_temp_ = v; }
  void SetTargetRh(float v) { target_rh_ = v; }
  void SetCalibration(const CalibrationData& v) { calib_ = v; }
  void SetCalibBmeT(float v) { calib_bme_t_ = v; }
  void SetCalibBmeH(float v) { calib_bme_h_ = v; }
  void SetCalibHtuT(float v) { calib_htu_t_ = v; }
  void SetCalibHtuH(float v) { calib_htu_h_ = v; }
  void SetCalibDsT(float v) { calib_ds_t_ = v; }
  void SetStats(const SystemStatistics& v) { stats_ = v; }
  void SetFanOn(bool v) { fan_on_ = v; }
  void SetOzoneOn(bool v) { ozone_on_ = v; }
  void SetManualMode(bool v) { is_manual_mode_ = v; }
  void SetAutoMode(bool v) { is_auto_mode_ = v; }

 private:
  SensorData inside_;
  SensorData outside_;
  SystemState state_;
  ErrorCode error_;

  float target_temp_;
  float target_rh_;

  CalibrationData calib_;
  float calib_bme_t_;
  float calib_bme_h_;
  float calib_htu_t_;
  float calib_htu_h_;
  float calib_ds_t_;

  SystemStatistics stats_;

  bool fan_on_;
  bool ozone_on_;

  // Дополнительные флаги для UI
  bool is_manual_mode_;
  bool is_auto_mode_;
};

#endif
