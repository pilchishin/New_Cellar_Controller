#include "MenuActions.h"
#include "DisplayUI.h"
#include "MenuNavigator.h"

/**
 * @brief Центральный исполнитель команд меню.
 * Выполняет диспетчеризацию ActionID на конкретные вызовы API контроллера или навигатора.
 */
void MenuActions::Execute(DisplayUI* ui, ActionID action) {
  if (action == ActionID::kNone) return;

  const MenuRootDef* root = ui->GetCurrentRootDef();

  switch (action) {
    case ActionID::kNavNext:
      // Переход к следующему пункту в текущем подразделе
      if (root) ui->nav_.NextItem(root->item_count);
      break;
    case ActionID::kNavPrev:
      // Переход к предыдущему пункту в текущем подразделе
      if (root) ui->nav_.PrevItem(root->item_count);
      break;
    case ActionID::kNavNextRoot:
      // Листание главных разделов вперед
      ui->nav_.NextRoot(ui->GetMenuTableSize());
      break;
    case ActionID::kNavPrevRoot:
      // Листание главных разделов назад
      ui->nav_.PrevRoot(ui->GetMenuTableSize());
      break;
    case ActionID::kValueInc:
      HandleUpValue(ui);
      break;
    case ActionID::kValueDec:
      HandleDownValue(ui);
      break;
    case ActionID::kEnterSubmenu:
      ui->nav_.EnterSubmenu();
      break;
    case ActionID::kExitSubmenu:
      ui->nav_.ExitSubmenu();
      break;
    case ActionID::kManualStart:
      HandleMenuManual(ui);
      break;
    case ActionID::kStatsReset:
      HandleLongMenuStats(ui);
      break;
    case ActionID::kErrorReset:
      // Команда контроллеру на попытку выхода из состояния ошибки
      ui->controller_->ResetError();
      break;
    default:
      break;
  }
}

/**
 * @brief Локальная утилита для чтения конфигурации страницы значения из Flash.
 */
static void LoadValuePageDef(uint8_t index, ValuePageDef* ram_buf) {
  if (ram_buf) {
    memcpy_P(ram_buf, &VALUE_PAGES[index], sizeof(ValuePageDef));
  }
}

/**
 * @brief Применение измененного значения к ядру системы.
 * Распределяет значения по соответствующим методам контроллера (уставки или калибровка).
 */
void MenuActions::ApplyValueChange(DisplayUI* ui, MenuItemID id, float val) {
  ValueID vid = ValueID::kNone;

  if (id == MenuItemID::kTargetTemp) {
    ui->controller_->SetTargetTemp(val);
    vid = ValueID::kTargetTemp;
  } else if (id == MenuItemID::kTargetHum) {
    ui->controller_->SetTargetRh(val);
    vid = ValueID::kTargetHum;
  } else if (id >= MenuItemID::kCalibBmeTemp && id <= MenuItemID::kCalibDsTemp) {
    // Групповая обработка калибровочных констант
    CalibrationData c = ui->model_.GetCalibration();
    if (id == MenuItemID::kCalibBmeTemp) { c.bmeTempOffset = val; vid = ValueID::kCalibBmeTemp; }
    else if (id == MenuItemID::kCalibBmeHum) { c.bmeHumOffset = val; vid = ValueID::kCalibBmeHum; }
    else if (id == MenuItemID::kCalibHtuTemp) { c.htuTempOffset = val; vid = ValueID::kCalibHtuTemp; }
    else if (id == MenuItemID::kCalibHtuHum) { c.htuHumOffset = val; vid = ValueID::kCalibHtuHum; }
    else if (id == MenuItemID::kCalibDsTemp) { c.dsTempOffset = val; vid = ValueID::kCalibDsTemp; }
    ui->controller_->SetCalibration(c);
  }

  // Обновляем локальную модель UI для мгновенного отображения без ожидания цикла синхронизации
  if (vid != ValueID::kNone) {
    ui->model_.SetValue(vid, val);
  }
}

/**
 * @brief Изменение значения с учетом шага и границ.
 */
void MenuActions::AdjustValue(DisplayUI* ui, float delta) {
  const MenuItemDef* item = ui->GetCurrentItemDef();
  if (!item) return;

  ValuePageDef vcfg;
  LoadValuePageDef(item->ctx_index, &vcfg);

  float val = ui->model_.GetValue(vcfg.val_id);
  val += delta;

  // Ограничение значения заданными в PROGMEM рамками
  if (val < vcfg.min_val) val = vcfg.min_val;
  if (val > vcfg.max_val) val = vcfg.max_val;

  ApplyValueChange(ui, item->id, val);
}

void MenuActions::HandleUpValue(DisplayUI* ui) {
  const MenuItemDef* item = ui->GetCurrentItemDef();
  if (!item) return;
  ValuePageDef vcfg;
  LoadValuePageDef(item->ctx_index, &vcfg);
  AdjustValue(ui, vcfg.step);
}

void MenuActions::HandleDownValue(DisplayUI* ui) {
  const MenuItemDef* item = ui->GetCurrentItemDef();
  if (!item) return;
  ValuePageDef vcfg;
  LoadValuePageDef(item->ctx_index, &vcfg);
  AdjustValue(ui, -vcfg.step);
}

/**
 * @brief Обработка запуска ручных режимов.
 * Устанавливает таймеры в контроллере и выводит подтверждающее сообщение.
 */
void MenuActions::HandleMenuManual(DisplayUI* ui) {
  if (ui->nav_.GetItemIndex() == 0) { // Элемент MANUAL_FAN
    ui->controller_->StartManualFan(30);
    ui->temp_message_ = "FAN STARTED";
  } else { // Элемент MANUAL_OZONE
    ui->controller_->StartManualOzone(15);
    ui->temp_message_ = "OZONE STARTED";
  }
  ui->message_timer_ = millis();
}

/**
 * @brief Сброс накопленной статистики наработки.
 * Требует нахождения на странице сброса внутри раздела STATS.
 */
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
