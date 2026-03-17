#include "MenuActions.h"
#include "DisplayUI.h"

// Переменные из DisplayUI.cpp, которые нужны здесь для идентификации калибровок
// В идеале их тоже стоит перенести в MenuActions или сделать общими.
// Для простоты пока объявим как внешние или перенесем.

void MenuActions::HandleDrawStatusIn(DisplayUI* ui) { ui->DrawStatusIn(); }
void MenuActions::HandleDrawStatusOut(DisplayUI* ui) { ui->DrawStatusOut(); }
void MenuActions::HandleDrawTargets(DisplayUI* ui) { ui->DrawTargets(); }
void MenuActions::HandleDrawManualModes(DisplayUI* ui) { ui->DrawManualModes(); }
void MenuActions::HandleDrawStats(DisplayUI* ui) { ui->DrawStats(); }
void MenuActions::HandleDrawErrorLog(DisplayUI* ui) { ui->DrawErrorLog(); }

void MenuActions::HandlePrevItem(DisplayUI* ui) {
  const MenuRootDef* root = ui->GetCurrentRootDef();
  if (root) {
    ui->nav_.PrevItem(root->item_count);
  }
}

void MenuActions::HandleNextItem(DisplayUI* ui) {
  const MenuRootDef* root = ui->GetCurrentRootDef();
  if (root) {
    ui->nav_.NextItem(root->item_count);
  }
}

void MenuActions::HandleDrawCalib(DisplayUI* ui) {
  const MenuItemDef* item = ui->GetCurrentItemDef();
  if (!item) return;

  CalibrationData c = ui->model_.calib;
  bool is_temp = true;
  float* val = ui->GetCalibrationParam(item->id, c, &is_temp);

  if (val) {
    ui->DrawCalibPage(item->label, *val, is_temp);
  }
}

void MenuActions::HandleUpTargets(DisplayUI* ui) {
  if (ui->nav_.GetItemIndex() == 0) // Температура
    ui->controller_->SetTargetTemp(ui->model_.target_temp + 0.1f);
  else { // Влажность
    float h = ui->model_.target_rh + 1.0f;
    if (h > 100.0f) h = 100.0f;
    ui->controller_->SetTargetRh(h);
  }
}

void MenuActions::HandleDownTargets(DisplayUI* ui) {
  if (ui->nav_.GetItemIndex() == 0) // Температура
    ui->controller_->SetTargetTemp(ui->model_.target_temp - 0.1f);
  else { // Влажность
    float h = ui->model_.target_rh - 1.0f;
    if (h < 0.0f) h = 0.0f;
    ui->controller_->SetTargetRh(h);
  }
}

void MenuActions::HandleUpManual(DisplayUI* ui) { HandlePrevItem(ui); }
void MenuActions::HandleDownManual(DisplayUI* ui) { HandleNextItem(ui); }

void MenuActions::HandleMenuManual(DisplayUI* ui) {
  if (ui->nav_.GetItemIndex() == 0) { // Вентилятор
    ui->controller_->StartManualFan(30);
    ui->temp_message_ = "FAN STARTED";
  } else { // Озон
    ui->controller_->StartManualOzone(15);
    ui->temp_message_ = "OZONE STARTED";
  }
  ui->message_timer_ = millis();
}

void MenuActions::HandleLongMenuStats(DisplayUI* ui) {
  if (ui->nav_.GetItemIndex() == 1) { // Страница STATS_RESET
    ui->controller_->ResetStats();
    ui->temp_message_ = "STATS RESET";
    ui->message_timer_ = millis();
    ui->nav_.ExitSubmenu();
  } else {
    ui->nav_.ExitSubmenu();
  }
}

void MenuActions::HandleUpError(DisplayUI* ui) { ui->controller_->ResetError(); }

void MenuActions::HandleUpCalib(DisplayUI* ui) { ui->AdjustCalib(0.1f); }
void MenuActions::HandleDownCalib(DisplayUI* ui) { ui->AdjustCalib(-0.1f); }

void MenuActions::HandlePrevRoot(DisplayUI* ui) {
  ui->nav_.PrevRoot(ui->GetMenuTableSize());
}

void MenuActions::HandleNextRoot(DisplayUI* ui) {
  ui->nav_.NextRoot(ui->GetMenuTableSize());
}

void MenuActions::HandleEnterSubmenu(DisplayUI* ui) {
  ui->nav_.EnterSubmenu();
}

void MenuActions::HandleExitSubmenu(DisplayUI* ui) {
  ui->nav_.ExitSubmenu();
}
