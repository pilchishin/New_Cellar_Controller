#include "DisplayUI.h"
#include "config.h"

// --- Статические обработчики MenuItemDef ---

void DisplayUI::HandleDrawStatusIn(DisplayUI* ui) { ui->DrawStatusIn(); }
void DisplayUI::HandleDrawStatusOut(DisplayUI* ui) { ui->DrawStatusOut(); }
void DisplayUI::HandleDrawTargets(DisplayUI* ui) { ui->DrawTargets(); }
void DisplayUI::HandleDrawManualModes(DisplayUI* ui) { ui->DrawManualModes(); }
void DisplayUI::HandleDrawStats(DisplayUI* ui) { ui->DrawStats(); }
void DisplayUI::HandleDrawErrorLog(DisplayUI* ui) { ui->DrawErrorLog(); }

void DisplayUI::HandleUpStatus(DisplayUI* ui) { ui->PrevItem(); }
void DisplayUI::HandleDownStatus(DisplayUI* ui) { ui->NextItem(); }

void DisplayUI::HandleDrawCalib(DisplayUI* ui) {
  CalibrationData c = ui->controller_->GetCalibration();
  switch (ui->current_item_) {
    case MenuItem::CALIB_BME_T: ui->DrawCalibPage("BME TEMP", c.bmeTempOffset, true); break;
    case MenuItem::CALIB_BME_H: ui->DrawCalibPage("BME HUM", c.bmeHumOffset, false); break;
    case MenuItem::CALIB_HTU_T: ui->DrawCalibPage("HTU TEMP", c.htuTempOffset, true); break;
    case MenuItem::CALIB_HTU_H: ui->DrawCalibPage("HTU HUM", c.htuHumOffset, false); break;
    case MenuItem::CALIB_DS_T:  ui->DrawCalibPage("DS TEMP", c.dsTempOffset, true); break;
    default: break;
  }
}

void DisplayUI::HandleUpTargets(DisplayUI* ui) {
  if (ui->current_item_ == MenuItem::TARGET_TEMP)
    ui->controller_->SetTargetTemp(ui->controller_->GetTargetTemp() + 0.1f);
  else {
    float h = ui->controller_->GetTargetRh() + 1.0f;
    if (h > 100.0f) h = 100.0f;
    ui->controller_->SetTargetRh(h);
  }
}
void DisplayUI::HandleDownTargets(DisplayUI* ui) {
  if (ui->current_item_ == MenuItem::TARGET_TEMP)
    ui->controller_->SetTargetTemp(ui->controller_->GetTargetTemp() - 0.1f);
  else {
    float h = ui->controller_->GetTargetRh() - 1.0f;
    if (h < 0.0f) h = 0.0f;
    ui->controller_->SetTargetRh(h);
  }
}

void DisplayUI::HandleUpManual(DisplayUI* ui) { ui->current_item_ = MenuItem::MANUAL_FAN; }
void DisplayUI::HandleDownManual(DisplayUI* ui) { ui->current_item_ = MenuItem::MANUAL_OZONE; }

void DisplayUI::HandleMenuManual(DisplayUI* ui) {
  if (ui->current_item_ == MenuItem::MANUAL_FAN) {
    ui->controller_->StartManualFan(30);
    ui->temp_message_ = "FAN STARTED";
  } else {
    ui->controller_->StartManualOzone(15);
    ui->temp_message_ = "OZONE STARTED";
  }
  ui->message_timer_ = millis();
}

void DisplayUI::HandleUpError(DisplayUI* ui) { ui->controller_->ResetError(); }

void DisplayUI::HandleUpCalib(DisplayUI* ui) {
  CalibrationData c = ui->controller_->GetCalibration();
  float* val = nullptr;
  switch (ui->current_item_) {
    case MenuItem::CALIB_BME_T: val = &c.bmeTempOffset; break;
    case MenuItem::CALIB_BME_H: val = &c.bmeHumOffset; break;
    case MenuItem::CALIB_HTU_T: val = &c.htuTempOffset; break;
    case MenuItem::CALIB_HTU_H: val = &c.htuHumOffset; break;
    case MenuItem::CALIB_DS_T:  val = &c.dsTempOffset; break;
    default: break;
  }
  if (val) {
    *val += 0.1f;
    if (*val > 5.0f) *val = 5.0f;
    ui->controller_->SetCalibration(c);
  }
}

void DisplayUI::HandleDownCalib(DisplayUI* ui) {
  CalibrationData c = ui->controller_->GetCalibration();
  float* val = nullptr;
  switch (ui->current_item_) {
    case MenuItem::CALIB_BME_T: val = &c.bmeTempOffset; break;
    case MenuItem::CALIB_BME_H: val = &c.bmeHumOffset; break;
    case MenuItem::CALIB_HTU_T: val = &c.htuTempOffset; break;
    case MenuItem::CALIB_HTU_H: val = &c.htuHumOffset; break;
    case MenuItem::CALIB_DS_T:  val = &c.dsTempOffset; break;
    default: break;
  }
  if (val) {
    *val -= 0.1f;
    if (*val < -5.0f) *val = -5.0f;
    ui->controller_->SetCalibration(c);
  }
}

static const MenuItemDef STATUS_ITEMS[] = {
  { MenuItem::STATUS_IN,   "STATUS_IN",   DisplayUI::HandleDrawStatusIn,  DisplayUI::HandleUpStatus, DisplayUI::HandleDownStatus, nullptr },
  { MenuItem::STATUS_OUT,  "STATUS_OUT",  DisplayUI::HandleDrawStatusOut, DisplayUI::HandleUpStatus, DisplayUI::HandleDownStatus, nullptr }
};

static const MenuItemDef kTargetsItems[] = {
  { MenuItem::TARGET_TEMP, "TARGET_TEMP", DisplayUI::HandleDrawTargets,   DisplayUI::HandleUpTargets, DisplayUI::HandleDownTargets, nullptr },
  { MenuItem::TARGET_HUM,  "TARGET_HUM",  DisplayUI::HandleDrawTargets,   DisplayUI::HandleUpTargets, DisplayUI::HandleDownTargets, nullptr }
};

static const MenuItemDef kManualItems[] = {
  { MenuItem::MANUAL_FAN,  "MANUAL_FAN",  DisplayUI::HandleDrawManualModes, DisplayUI::HandleUpManual, DisplayUI::HandleDownManual, DisplayUI::HandleMenuManual },
  { MenuItem::MANUAL_OZONE,"MANUAL_OZONE",DisplayUI::HandleDrawManualModes, DisplayUI::HandleUpManual, DisplayUI::HandleDownManual, DisplayUI::HandleMenuManual }
};

static const MenuItemDef kStatsItems[] = {
  { MenuItem::STATS_VIEW,  "STATS_VIEW",  DisplayUI::HandleDrawStats,     nullptr,            nullptr,              nullptr },
  { MenuItem::STATS_RESET, "STATS_RESET", DisplayUI::HandleDrawStats,     nullptr,            nullptr,              nullptr }
};

static const MenuItemDef kErrorItems[] = {
  { MenuItem::ERROR_VIEW,  "ERROR_VIEW",  DisplayUI::HandleDrawErrorLog,  DisplayUI::HandleUpError, nullptr,             nullptr }
};

static const MenuItemDef kServiceItems[] = {
  { MenuItem::CALIB_BME_T, "CALIB_BME_T", DisplayUI::HandleDrawCalib,     DisplayUI::HandleUpCalib, DisplayUI::HandleDownCalib, nullptr },
  { MenuItem::CALIB_BME_H, "CALIB_BME_H", DisplayUI::HandleDrawCalib,     DisplayUI::HandleUpCalib, DisplayUI::HandleDownCalib, nullptr },
  { MenuItem::CALIB_HTU_T, "CALIB_HTU_T", DisplayUI::HandleDrawCalib,     DisplayUI::HandleUpCalib, DisplayUI::HandleDownCalib, nullptr },
  { MenuItem::CALIB_HTU_H, "CALIB_HTU_H", DisplayUI::HandleDrawCalib,     DisplayUI::HandleUpCalib, DisplayUI::HandleDownCalib, nullptr },
  { MenuItem::CALIB_DS_T,  "CALIB_DS_T",  DisplayUI::HandleDrawCalib,     DisplayUI::HandleUpCalib, DisplayUI::HandleDownCalib, nullptr }
};

static const MenuRootDef MENU_TABLE[] = {
  { MenuRoot::HOME,    "HOME",    nullptr,       0 },
  { MenuRoot::STATUS,  "STATUS",  STATUS_ITEMS,  2 },
  { MenuRoot::TARGETS, "TARGETS", kTargetsItems, 2 },
  { MenuRoot::MANUAL,  "MANUAL",  kManualItems,  2 },
  { MenuRoot::STATS,   "STATS",   kStatsItems,   2 },
  { MenuRoot::ERRORS,  "ERRORS",  kErrorItems,   1 },
  { MenuRoot::SERVICE, "SERVICE", kServiceItems, 5 }
};

const MenuRootDef* DisplayUI::FindRootDef(MenuRoot id) {
  for (uint8_t i = 0; i < sizeof(MENU_TABLE) / sizeof(MENU_TABLE[0]); i++) {
    if (MENU_TABLE[i].id == id) return &MENU_TABLE[i];
  }
  return nullptr;
}

const MenuItemDef* DisplayUI::FindItemDef(MenuItem id) {
  const MenuRootDef* root = FindRootDef(current_root_);
  if (!root || !root->items) return nullptr;
  for (uint8_t i = 0; i < root->item_count; i++) {
    if (root->items[i].id == id) return &root->items[i];
  }
  return nullptr;
}

// Конструктор: адрес 0x27 и размер 16x2
DisplayUI::DisplayUI(Controller* c, SensorManager* s, TimeManager* t)
    : lcd_(0x27, 16, 2),
      controller_(c),
      sensors_(s),
      rtc_(t),
      current_root_(MenuRoot::HOME),
      current_item_(MenuItem::NONE),
      in_submenu_(false),
      submenu_index_(0),
      submenu_count_(0),
      last_btn_check_(0),
      last_btn_action_(0),
      menu_btn_timer_(0),
      menu_btn_pressed_(false),
      message_timer_(0),
      temp_message_(nullptr),
      last_activity_time_(0),
      backlight_on_(true),
      needs_redraw_(true) {
  memset(last_lines_, 0, sizeof(last_lines_));
}

void DisplayUI::Init() {
  lcd_.init();
  lcd_.backlight();
  pinMode(BT_UP, INPUT_PULLUP);
  pinMode(BT_DOWN, INPUT_PULLUP);
  pinMode(BT_MENU, INPUT_PULLUP);
  last_activity_time_ = millis();
}

void DisplayUI::Reinit() {
  lcd_.init();
  if (backlight_on_)
    lcd_.backlight();
  else
    lcd_.noBacklight();
}

void DisplayUI::Update() {
  HandleButtons();    // Опрос кнопок
  UpdateBacklight();  // Управление светом

  static unsigned long last_draw = 0;
  unsigned long interval = (current_root_ == MenuRoot::HOME && !in_submenu_) ? 500 : 1000;

  if (needs_redraw_ || (millis() - last_draw >= interval)) {
    last_draw = millis();
    needs_redraw_ = false;

    // Сбрасываем буферы
    screen_.Clear();

    // Если отображается временное сообщение, заполняем буферы им
    if (temp_message_ != nullptr && millis() - message_timer_ < 2000) {
      screen_.SetPos(0, 0);
      screen_.print(temp_message_);
      screen_.SetPos(1, 0);
      screen_.print(F("                "));
    } else {
      if (temp_message_ != nullptr) {
        temp_message_ = nullptr;  // Сброс сообщения по истечении времени
      }
      DrawPage(); // DrawPage теперь наполняет screen_
    }

    Flush(); // Flush сравнивает с last_lines_ и выводит только изменения
  }
}

void DisplayUI::Flush() {
  for (int i = 0; i < 2; i++) {
    const char* line = screen_.GetLine(i);
    if (strcmp(line, last_lines_[i]) != 0) {
      lcd_.setCursor(0, i);
      lcd_.print(line);
      strcpy(last_lines_[i], line);
    }
  }
}

/**
 * @brief Опрос кнопок и обработка нажатий (UP, DOWN, MENU).
 * Использует неблокирующий антидребезг и таймеры повтора.
 */
void DisplayUI::HandleButtons() {
  if (millis() - last_btn_check_ < 50) return;  // Базовая задержка антидребезга
  last_btn_check_ = millis();

  // Считывание состояний кнопок (инвертировано из-за INPUT_PULLUP)
  bool up = !digitalRead(BT_UP);
  bool down = !digitalRead(BT_DOWN);
  bool menu = !digitalRead(BT_MENU);

  // Сброс таймера гашения подсветки при любой активности
  if (up || down || menu) {
    needs_redraw_ = true;
    controller_->NotifyUserActivity();  // Уведомляем контроллер о присутствии человека
    last_activity_time_ = millis();
    if (!backlight_on_) {
      lcd_.backlight();
      backlight_on_ = true;
      return;  // Первое нажатие при выключенном экране только включает свет
    }
  }

  // Логика кнопки MENU
  if (menu) {
    if (!menu_btn_pressed_) {
      menu_btn_pressed_ = true;
      menu_btn_timer_ = millis();
    }
  } else {
    if (menu_btn_pressed_) {
      unsigned long press_duration = millis() - menu_btn_timer_;
      if (press_duration < 600) {
        // Короткое нажатие
        if (!in_submenu_) {
          // если не в submenu: переключать MenuRoot
          NextRoot();
        } else {
          // если в submenu: переключать MenuItem внутри раздела
          const MenuItemDef* def = FindItemDef(current_item_);
          if (def && def->on_menu) {
            def->on_menu(this);
            needs_redraw_ = true;
          } else {
            NextItem();
          }
        }
      } else {
        // Длинное нажатие
        if (!in_submenu_) {
          // если submenu закрыт: открыть submenu выбранного раздела
          if (current_root_ != MenuRoot::HOME) {
            in_submenu_ = true;
          }
        } else {
          // если submenu открыт: выйти в root меню
          if (current_item_ == MenuItem::STATS_RESET) {
            controller_->ResetStats();
            temp_message_ = "STATS RESET";
            message_timer_ = millis();
          }
          in_submenu_ = false;
        }
      }
      menu_btn_pressed_ = false;
    }
  }

  // Логика кнопок изменения значений (UP/DOWN)
  if ((up || down) && !menu_btn_pressed_) {
    // Ограничение скорости изменения значений (150мс между шагами)
    if (millis() - last_btn_action_ >= 150) {
      last_btn_action_ = millis();

      if (in_submenu_) {
        const MenuItemDef* def = FindItemDef(current_item_);
        if (def) {
          if (up && def->on_up) def->on_up(this);
          if (down && def->on_down) def->on_down(this);
          needs_redraw_ = true;
        }
      }
    }
  }
}

void DisplayUI::NextRoot() {
  int idx = -1;
  for (uint8_t i = 0; i < sizeof(MENU_TABLE) / sizeof(MENU_TABLE[0]); i++) {
    if (MENU_TABLE[i].id == current_root_) {
      idx = i;
      break;
    }
  }

  idx = (idx + 1) % (sizeof(MENU_TABLE) / sizeof(MENU_TABLE[0]));
  current_root_ = MENU_TABLE[idx].id;

  in_submenu_ = false;
  SetDefaultItemForRoot();
  needs_redraw_ = true;
}

void DisplayUI::NextItem() {
  const MenuRootDef* root = FindRootDef(current_root_);
  if (!root || root->item_count == 0) return;

  int idx = -1;
  for (uint8_t i = 0; i < root->item_count; i++) {
    if (root->items[i].id == current_item_) {
      idx = i;
      break;
    }
  }

  idx = (idx + 1) % root->item_count;
  current_item_ = root->items[idx].id;

  UpdateSubmenuIndex();
  needs_redraw_ = true;
}

void DisplayUI::PrevItem() {
  const MenuRootDef* root = FindRootDef(current_root_);
  if (!root || root->item_count == 0) return;

  int idx = -1;
  for (uint8_t i = 0; i < root->item_count; i++) {
    if (root->items[i].id == current_item_) {
      idx = i;
      break;
    }
  }

  idx = (idx - 1 + root->item_count) % root->item_count;
  current_item_ = root->items[idx].id;

  UpdateSubmenuIndex();
  needs_redraw_ = true;
}

void DisplayUI::SetDefaultItemForRoot() {
  const MenuRootDef* root = FindRootDef(current_root_);
  if (root && root->item_count > 0) {
    current_item_ = root->items[0].id;
  } else {
    current_item_ = MenuItem::NONE;
  }
  UpdateSubmenuIndex();
  needs_redraw_ = true;
}

void DisplayUI::UpdateSubmenuIndex() {
  const MenuRootDef* root = FindRootDef(current_root_);
  if (!root || root->item_count == 0) {
    submenu_index_ = 0;
    submenu_count_ = 0;
    return;
  }

  submenu_count_ = root->item_count;
  for (uint8_t i = 0; i < root->item_count; i++) {
    if (root->items[i].id == current_item_) {
      submenu_index_ = i + 1;
      break;
    }
  }
}

void DisplayUI::UpdateBacklight() {
  // Если прошло более 30 секунд бездействия - гасим свет
  // ИСКЛЮЧЕНИЕ: Manual Ozone (по ТЗ в ручном режиме подсветка может игнорироваться)
  if (backlight_on_ && (millis() - last_activity_time_ > 30000UL)) {
    if (controller_->GetState() != SystemState::kManualOzone) {
      lcd_.noBacklight();
      backlight_on_ = false;
    }
  }
}

void DisplayUI::DrawPage() {
  if (!in_submenu_) {
    DrawRootPage();
  } else {
    DrawSubPage();
  }
}

void DisplayUI::DrawRootPage() {
  if (current_root_ == MenuRoot::HOME) {
    DrawHomeScreen();
    return;
  }

  const MenuRootDef* root = FindRootDef(current_root_);
  screen_.SetPos(0, 0);
  screen_.print(F("> "));
  if (root) screen_.print(root->label);

  screen_.SetPos(1, 0);
  screen_.print(F("  MENU ENTER"));
}

void DisplayUI::DrawSubPage() {
  const MenuItemDef* def = FindItemDef(current_item_);
  if (def && def->draw) {
    def->draw(this);
  }
}

void DisplayUI::DrawHomeScreen() {
  SensorData in = sensors_->GetInside();
  SensorData out = sensors_->GetOutside();
  RelayManager* rm = controller_->GetRelayManager();

  // Строка 1: IN temp humidity fan/ozone indicator
  screen_.SetPos(0, 0);
  screen_.print(F("IN "));
  screen_.print(in.temp, 1);
  screen_.print(F("C "));
  screen_.print(in.rh, 0);
  screen_.print(F("%"));

  screen_.SetPos(0, 15);
  if (rm->GetOzoneState())
    screen_.print(F("O"));
  else if (rm->GetFanState())
    screen_.print(F("F"));
  else
    screen_.print(F(" "));

  // Строка 2: OUT temp humidity режим системы
  screen_.SetPos(1, 0);
  screen_.print(F("OUT "));
  screen_.print(out.temp, 1);
  screen_.print(F("C "));
  screen_.print(out.rh, 0);
  screen_.print(F("%"));

  screen_.SetPos(1, 15);
  SystemState state = controller_->GetState();
  if (state == SystemState::kErrorState)
    screen_.print(F("E"));
  else if (state == SystemState::kAutoClimate ||
           state == SystemState::kOzoneStart ||
           state == SystemState::kOzoneActive ||
           state == SystemState::kOzoneHold ||
           state == SystemState::kOzoneVent)
    screen_.print(F("A"));
  else
    screen_.print(F("M"));
}

void DisplayUI::DrawStatusIn() {
  screen_.SetPos(0, 0);
  screen_.print(F("STATUS "));
  screen_.print(submenu_index_);
  screen_.print(F("/"));
  screen_.print(submenu_count_);

  SensorData in = sensors_->GetInside();
  screen_.SetPos(1, 0);
  screen_.print(F("IN "));
  screen_.print(in.temp, 1);
  screen_.print(F("C "));
  screen_.print(in.rh, 0);
  screen_.print(F("%"));
}

void DisplayUI::DrawStatusOut() {
  screen_.SetPos(0, 0);
  screen_.print(F("STATUS "));
  screen_.print(submenu_index_);
  screen_.print(F("/"));
  screen_.print(submenu_count_);

  SensorData out = sensors_->GetOutside();
  screen_.SetPos(1, 0);
  screen_.print(F("OUT "));
  screen_.print(out.temp, 1);
  screen_.print(F("C "));
  screen_.print(out.rh, 0);
  screen_.print(F("%"));
}

void DisplayUI::DrawTargets() {
  screen_.SetPos(0, 0);
  screen_.print(F("TARGETS "));
  screen_.print(submenu_index_);
  screen_.print(F("/"));
  screen_.print(submenu_count_);

  screen_.SetPos(1, 0);
  if (current_item_ == MenuItem::TARGET_TEMP)
    screen_.print(F(">"));
  else
    screen_.print(F(" "));
  screen_.print(F("T:"));
  screen_.print(controller_->GetTargetTemp(), 1);

  screen_.print(F(" "));
  if (current_item_ == MenuItem::TARGET_HUM)
    screen_.print(F(">"));
  else
    screen_.print(F(" "));
  screen_.print(F("H:"));
  screen_.print(controller_->GetTargetRh(), 0);
  screen_.print(F("%"));
}

void DisplayUI::DrawManualModes() {
  screen_.SetPos(0, 0);
  screen_.print(F("MANUAL "));
  screen_.print(submenu_index_);
  screen_.print(F("/"));
  screen_.print(submenu_count_);

  screen_.SetPos(1, 0);
  if (current_item_ == MenuItem::MANUAL_FAN)
    screen_.print(F(">"));
  else
    screen_.print(F(" "));
  screen_.print(F("FAN   "));

  if (current_item_ == MenuItem::MANUAL_OZONE)
    screen_.print(F(">"));
  else
    screen_.print(F(" "));
  screen_.print(F("OZONE"));
}

void DisplayUI::DrawCalibPage(const char* label, float value, bool is_temp) {
  screen_.SetPos(0, 0);
  screen_.print(F("SERVICE "));
  screen_.print(submenu_index_);
  screen_.print(F("/"));
  screen_.print(submenu_count_);

  screen_.SetPos(1, 0);
  screen_.print(label);
  screen_.print(F(" "));
  if (value >= 0) screen_.print(F("+"));
  screen_.print(value, 1);
  screen_.print(is_temp ? F("C") : F("%"));
}

void DisplayUI::DrawStats() {
  screen_.SetPos(0, 0);
  screen_.print(F("STATS "));
  screen_.print(submenu_index_);
  screen_.print(F("/"));
  screen_.print(submenu_count_);

  screen_.SetPos(1, 0);
  if (current_item_ == MenuItem::STATS_VIEW) {
    SystemStatistics s = controller_->GetStats();
    screen_.print(F("U:"));
    screen_.print(s.uptimeMinutes / 60);
    screen_.print(F(" F:"));
    screen_.print(s.fanMinutes / 60);
    screen_.print(F(" O3:"));
    screen_.print(s.ozoneMinutes / 60);
  } else if (current_item_ == MenuItem::STATS_RESET) {
    screen_.print(F("MENU CONFIRM"));
  }
}

void DisplayUI::DrawErrorLog() {
  screen_.SetPos(0, 0);
  screen_.print(F("ERRORS "));
  screen_.print(submenu_index_);
  screen_.print(F("/"));
  screen_.print(submenu_count_);

  screen_.SetPos(1, 0);
  ErrorCode err = controller_->GetError();
  if (err == ErrorCode::kNone) {
    screen_.print(F("SYSTEM OK"));
  } else {
    screen_.print(ErrorToString(err));
    screen_.SetPos(1, 12);
    screen_.print(F("UP:R"));
  }
}
