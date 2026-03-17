#include "UIModel.h"
#include "Controller.h"
#include "SensorManager.h"

UIModel::UIModel()
    : inside{0,0,0,0,false},
      outside{0,0,0,0,false},
      state(SystemState::kIdle),
      error(ErrorCode::kNone),
      target_temp(0),
      target_rh(0),
      calib{0,0,0,0,0},
      calib_bme_t(0),
      calib_bme_h(0),
      calib_htu_t(0),
      calib_htu_h(0),
      calib_ds_t(0),
      stats{0,0,0},
      fan_on(false),
      ozone_on(false),
      is_manual_mode(false),
      is_auto_mode(false) {
}

void UIModel::Sync(Controller* controller, SensorManager* sensors) {
  if (!controller || !sensors) return;

  inside = sensors->GetInside();
  outside = sensors->GetOutside();
  state = controller->GetState();
  error = controller->GetError();

  target_temp = controller->GetTargetTemp();
  target_rh = controller->GetTargetRh();

  calib = controller->GetCalibration();
  calib_bme_t = calib.bmeTempOffset;
  calib_bme_h = calib.bmeHumOffset;
  calib_htu_t = calib.htuTempOffset;
  calib_htu_h = calib.htuHumOffset;
  calib_ds_t = calib.dsTempOffset;

  stats = controller->GetStats();

  RelayManager* rm = controller->GetRelayManager();
  if (rm) {
    fan_on = rm->GetFanState();
    ozone_on = rm->GetOzoneState();
  }

  is_manual_mode = (state == SystemState::kManualFan || state == SystemState::kManualOzone);
  is_auto_mode = (state == SystemState::kAutoClimate ||
                  state == SystemState::kOzoneStart ||
                  state == SystemState::kOzoneActive ||
                  state == SystemState::kOzoneHold ||
                  state == SystemState::kOzoneVent);
}
