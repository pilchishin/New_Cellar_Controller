#include "MenuActions.h"
#include "DisplayUI.h"
#include "MenuNavigator.h"

// Переменные из DisplayUI.cpp, которые нужны здесь для идентификации калибровок
// В идеале их тоже стоит перенести в MenuActions или сделать общими.
// Для простоты пока объявим как внешние или перенесем.

void MenuActions::HandleDrawStatus(DisplayUI* ui) {
  const MenuItemDef* item = ui->GetCurrentItemDef();
  if (item) ui->DrawStatus((int)item->ctx);
}

void MenuActions::HandleDrawManualModes(DisplayUI* ui) { ui->DrawManualModes(); }
void MenuActions::HandleDrawStats(DisplayUI* ui) { ui->DrawStats(); }
void MenuActions::HandleDrawErrorLog(DisplayUI* ui) { ui->DrawErrorLog(); }
void MenuActions::HandleDrawValue(DisplayUI* ui) { ui->DrawValuePage(); }

static void LoadValuePageDef(const void* flash_ptr, ValuePageDef* ram_buf) {
  if (flash_ptr && ram_buf) {
    memcpy_P(ram_buf, flash_ptr, sizeof(ValuePageDef));
  }
}

void MenuActions::ApplyValueChange(DisplayUI* ui, MenuItemID id, float val) {
  ValueID vid = ValueID::kNone;

  if (id == MenuItemID::kTargetTemp) {
    ui->controller_->SetTargetTemp(val);
    vid = ValueID::kTargetTemp;
  } else if (id == MenuItemID::kTargetHum) {
    ui->controller_->SetTargetRh(val);
    vid = ValueID::kTargetHum;
  } else if (id >= MenuItemID::kCalibBmeTemp && id <= MenuItemID::kCalibDsTemp) {
    CalibrationData c = ui->model_.calib;
    if (id == MenuItemID::kCalibBmeTemp) { c.bmeTempOffset = val; vid = ValueID::kCalibBmeTemp; }
    else if (id == MenuItemID::kCalibBmeHum) { c.bmeHumOffset = val; vid = ValueID::kCalibBmeHum; }
    else if (id == MenuItemID::kCalibHtuTemp) { c.htuTempOffset = val; vid = ValueID::kCalibHtuTemp; }
    else if (id == MenuItemID::kCalibHtuHum) { c.htuHumOffset = val; vid = ValueID::kCalibHtuHum; }
    else if (id == MenuItemID::kCalibDsTemp) { c.dsTempOffset = val; vid = ValueID::kCalibDsTemp; }
    ui->controller_->SetCalibration(c);
  }

  if (vid != ValueID::kNone) {
    ui->model_.SetValue(vid, val);
  }
}

void MenuActions::AdjustValue(DisplayUI* ui, float delta) {
  const MenuItemDef* item = ui->GetCurrentItemDef();
  if (!item || !item->ctx) return;

  ValuePageDef vcfg;
  LoadValuePageDef(item->ctx, &vcfg);

  // Для калибровки используем точность 1 знак, для других (например HUM) берем из конфига
  float val = ui->model_.GetValue(vcfg.val_id);
  val += delta;

  if (val < vcfg.min_val) val = vcfg.min_val;
  if (val > vcfg.max_val) val = vcfg.max_val;

  ApplyValueChange(ui, item->id, val);
}

void MenuActions::HandleUpValue(DisplayUI* ui) {
  const MenuItemDef* item = ui->GetCurrentItemDef();
  if (!item || !item->ctx) return;
  ValuePageDef vcfg;
  LoadValuePageDef(item->ctx, &vcfg);
  AdjustValue(ui, vcfg.step);
}

void MenuActions::HandleDownValue(DisplayUI* ui) {
  const MenuItemDef* item = ui->GetCurrentItemDef();
  if (!item || !item->ctx) return;
  ValuePageDef vcfg;
  LoadValuePageDef(item->ctx, &vcfg);
  AdjustValue(ui, -vcfg.step);
}

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
