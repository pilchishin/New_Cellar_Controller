/**
 * @file UIModel.cpp
 * @brief Реализация локальной модели данных для пользовательского интерфейса.
 */

#include "UIModel.h"
#include "Controller.h"
#include "SensorManager.h"

/**
 * @brief Конструктор модели. Инициализирует все поля значениями по умолчанию.
 */
UIModel::UIModel()
    : inside_{0,0,0,0,false},
      outside_{0,0,0,0,false},
      state_(SystemState::kIdle),
      error_(ErrorCode::kNone),
      target_temp_(0),
      target_rh_(0),
      calib_{0,0,0,0,0},
      calib_bme_t_(0),
      calib_bme_h_(0),
      calib_htu_t_(0),
      calib_htu_h_(0),
      calib_ds_t_(0),
      stats_{0,0,0},
      fan_on_(false),
      ozone_on_(false),
      is_manual_mode_(false),
      is_auto_mode_(false) {
}

/**
 * @brief Синхронизация локальной модели с актуальным состоянием системы.
 * Извлекает данные из контроллера и менеджера датчиков.
 */
void UIModel::Sync(Controller* controller, SensorManager* sensors) {
  if (!controller || !sensors) return;

  SetInside(sensors->GetInside());
  SetOutside(sensors->GetOutside());
  SetState(controller->GetState());
  SetError(controller->GetError());

  SetTargetTemp(controller->GetTargetTemp());
  SetTargetRh(controller->GetTargetRh());

  SetCalibration(controller->GetCalibration());
  CalibrationData c = GetCalibration();
  SetCalibBmeT(c.bmeTempOffset);
  SetCalibBmeH(c.bmeHumOffset);
  SetCalibHtuT(c.htuTempOffset);
  SetCalibHtuH(c.htuHumOffset);
  SetCalibDsT(c.dsTempOffset);

  SetStats(controller->GetStats());

  RelayManager* rm = controller->GetRelayManager();
  if (rm) {
    SetFanOn(rm->GetFanState());
    SetOzoneOn(rm->GetOzoneState());
  }

  SetManualMode(GetState() == SystemState::kManualFan || GetState() == SystemState::kManualOzone);
  SetAutoMode(GetState() == SystemState::kAutoClimate ||
              GetState() == SystemState::kOzoneStart ||
              GetState() == SystemState::kOzoneActive ||
              GetState() == SystemState::kOzoneHold ||
              GetState() == SystemState::kOzoneVent);
}

/**
 * @brief Получение числового значения параметра по его идентификатору.
 */
float UIModel::GetValue(ValueID id) const {
  switch (id) {
    case ValueID::kTargetTemp:   return GetTargetTemp();
    case ValueID::kTargetHum:    return GetTargetRh();
    case ValueID::kCalibBmeTemp: return GetCalibBmeT();
    case ValueID::kCalibBmeHum:  return GetCalibBmeH();
    case ValueID::kCalibHtuTemp: return GetCalibHtuT();
    case ValueID::kCalibHtuHum:  return GetCalibHtuH();
    case ValueID::kCalibDsTemp:  return GetCalibDsT();
    default: return 0.0f;
  }
}

/**
 * @brief Локальное обновление значения параметра в модели.
 */
void UIModel::SetValue(ValueID id, float v) {
  switch (id) {
    case ValueID::kTargetTemp:   SetTargetTemp(v); break;
    case ValueID::kTargetHum:    SetTargetRh(v); break;
    case ValueID::kCalibBmeTemp: SetCalibBmeT(v); break;
    case ValueID::kCalibBmeHum:  SetCalibBmeH(v); break;
    case ValueID::kCalibHtuTemp: SetCalibHtuT(v); break;
    case ValueID::kCalibHtuHum:  SetCalibHtuH(v); break;
    case ValueID::kCalibDsTemp:  SetCalibDsT(v); break;
    default: break;
  }
}
