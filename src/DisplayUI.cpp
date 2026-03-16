#include "DisplayUI.h"
#include "config.h"

// --- Статические обработчики MenuItemDef ---

void DisplayUI::HandleDrawStatusIn(DisplayUI* ui) { ui->DrawStatusIn(); }
void DisplayUI::HandleDrawStatusOut(DisplayUI* ui) { ui->DrawStatusOut(); }
void DisplayUI::HandleDrawTargets(DisplayUI* ui) { ui->DrawTargets(); }
void DisplayUI::HandleDrawManualModes(DisplayUI* ui) { ui->DrawManualModes(); }
void DisplayUI::HandleDrawStats(DisplayUI* ui) { ui->DrawStats(); }
void DisplayUI::HandleDrawErrorLog(DisplayUI* ui) { ui->DrawErrorLog(); }

void DisplayUI::HandlePrevItem(DisplayUI* ui) {
  const MenuRootDef* root = ui->GetCurrentRootDef();
  if (root && root->item_count > 0) {
    ui->item_index_ = (ui->item_index_ + root->item_count - 1) % root->item_count;
  }
}
void DisplayUI::HandleNextItem(DisplayUI* ui) {
  const MenuRootDef* root = ui->GetCurrentRootDef();
  if (root && root->item_count > 0) {
    ui->item_index_ = (ui->item_index_ + 1) % root->item_count;
  }
}

void DisplayUI::HandleDrawCalib(DisplayUI* ui) {
  CalibrationData c = ui->controller_->GetCalibration();
  int idx = ui->item_index_;
  if (idx >= 0 && idx < 5) {
    float* offsets[] = { &c.bmeTempOffset, &c.bmeHumOffset, &c.htuTempOffset, &c.htuHumOffset, &c.dsTempOffset };
    const MenuItemDef* def = ui->GetCurrentItemDef();
    if (def) {
      ui->DrawCalibPage(def->label, *offsets[idx], (idx != 1 && idx != 3));
    }
  }
}

void DisplayUI::HandleUpTargets(DisplayUI* ui) {
  if (ui->item_index_ == 0) // TARGET_TEMP
    ui->controller_->SetTargetTemp(ui->controller_->GetTargetTemp() + 0.1f);
  else {
    float h = ui->controller_->GetTargetRh() + 1.0f;
    if (h > 100.0f) h = 100.0f;
    ui->controller_->SetTargetRh(h);
  }
}
void DisplayUI::HandleDownTargets(DisplayUI* ui) {
  if (ui->item_index_ == 0) // TARGET_TEMP
    ui->controller_->SetTargetTemp(ui->controller_->GetTargetTemp() - 0.1f);
  else {
    float h = ui->controller_->GetTargetRh() - 1.0f;
    if (h < 0.0f) h = 0.0f;
    ui->controller_->SetTargetRh(h);
  }
}

void DisplayUI::HandleUpManual(DisplayUI* ui) { ui->item_index_ = 0; }
void DisplayUI::HandleDownManual(DisplayUI* ui) { ui->item_index_ = 1; }

void DisplayUI::HandleMenuManual(DisplayUI* ui) {
  if (ui->item_index_ == 0) { // MANUAL_FAN
    ui->controller_->StartManualFan(30);
    ui->temp_message_ = "FAN STARTED";
  } else {
    ui->controller_->StartManualOzone(15);
    ui->temp_message_ = "OZONE STARTED";
  }
  ui->message_timer_ = millis();
}

void DisplayUI::HandleLongMenuStats(DisplayUI* ui) {
  if (ui->item_index_ == 1) { // STATS_RESET
    ui->controller_->ResetStats();
    ui->temp_message_ = "STATS RESET";
    ui->message_timer_ = millis();
    ui->in_submenu_ = false;
  } else {
    ui->in_submenu_ = false;
  }
}

void DisplayUI::HandleUpError(DisplayUI* ui) { ui->controller_->ResetError(); }

void DisplayUI::AdjustCalib(DisplayUI* ui, float delta) {
  CalibrationData c = ui->controller_->GetCalibration();
  int idx = ui->item_index_;
  if (idx >= 0 && idx < 5) {
    float* offsets[] = { &c.bmeTempOffset, &c.bmeHumOffset, &c.htuTempOffset, &c.htuHumOffset, &c.dsTempOffset };
    float* val = offsets[idx];
    *val += delta;
    if (*val > 5.0f) *val = 5.0f;
    if (*val < -5.0f) *val = -5.0f;
    ui->controller_->SetCalibration(c);
  }
}

void DisplayUI::HandleUpCalib(DisplayUI* ui) { AdjustCalib(ui, 0.1f); }
void DisplayUI::HandleDownCalib(DisplayUI* ui) { AdjustCalib(ui, -0.1f); }

void DisplayUI::HandleEnterSubmenu(DisplayUI* ui) {
  if (ui->root_index_ != 0) { // HOME
    ui->in_submenu_ = true;
  }
}
void DisplayUI::HandleExitSubmenu(DisplayUI* ui) { ui->in_submenu_ = false; }

static const MenuItemDef STATUS_ITEMS[] = {
  { MenuItem::STATUS_IN,   "STATUS_IN",   DisplayUI::HandleDrawStatusIn,  DisplayUI::HandlePrevItem, DisplayUI::HandleNextItem, DisplayUI::HandleNextItem, DisplayUI::HandleExitSubmenu },
  { MenuItem::STATUS_OUT,  "STATUS_OUT",  DisplayUI::HandleDrawStatusOut, DisplayUI::HandlePrevItem, DisplayUI::HandleNextItem, DisplayUI::HandleNextItem, DisplayUI::HandleExitSubmenu }
};

static const MenuItemDef TARGET_ITEMS[] = {
  { MenuItem::TARGET_TEMP, "TARGET_TEMP", DisplayUI::HandleDrawTargets,   DisplayUI::HandleUpTargets, DisplayUI::HandleDownTargets, DisplayUI::HandleNextItem, DisplayUI::HandleExitSubmenu },
  { MenuItem::TARGET_HUM,  "TARGET_HUM",  DisplayUI::HandleDrawTargets,   DisplayUI::HandleUpTargets, DisplayUI::HandleDownTargets, DisplayUI::HandleNextItem, DisplayUI::HandleExitSubmenu }
};

static const MenuItemDef MANUAL_ITEMS[] = {
  { MenuItem::MANUAL_FAN,  "MANUAL_FAN",  DisplayUI::HandleDrawManualModes, DisplayUI::HandleUpManual, DisplayUI::HandleDownManual, DisplayUI::HandleMenuManual, DisplayUI::HandleExitSubmenu },
  { MenuItem::MANUAL_OZONE,"MANUAL_OZONE",DisplayUI::HandleDrawManualModes, DisplayUI::HandleUpManual, DisplayUI::HandleDownManual, DisplayUI::HandleMenuManual, DisplayUI::HandleExitSubmenu }
};

static const MenuItemDef STATS_ITEMS[] = {
  { MenuItem::STATS_VIEW,  "STATS_VIEW",  DisplayUI::HandleDrawStats,     DisplayUI::HandlePrevItem, DisplayUI::HandleNextItem, DisplayUI::HandleNextItem, DisplayUI::HandleExitSubmenu },
  { MenuItem::STATS_RESET, "STATS_RESET", DisplayUI::HandleDrawStats,     DisplayUI::HandlePrevItem, DisplayUI::HandleNextItem, DisplayUI::HandleNextItem, DisplayUI::HandleLongMenuStats }
};

static const MenuItemDef ERROR_ITEMS[] = {
  { MenuItem::ERROR_VIEW,  "ERROR_VIEW",  DisplayUI::HandleDrawErrorLog,  DisplayUI::HandleUpError, nullptr,             DisplayUI::HandleNextItem, DisplayUI::HandleExitSubmenu }
};

static const MenuItemDef SERVICE_ITEMS[] = {
  { MenuItem::CALIB_BME_T, "BME TEMP", DisplayUI::HandleDrawCalib,     DisplayUI::HandleUpCalib, DisplayUI::HandleDownCalib, DisplayUI::HandleNextItem, DisplayUI::HandleExitSubmenu },
  { MenuItem::CALIB_BME_H, "BME HUM",  DisplayUI::HandleDrawCalib,     DisplayUI::HandleUpCalib, DisplayUI::HandleDownCalib, DisplayUI::HandleNextItem, DisplayUI::HandleExitSubmenu },
  { MenuItem::CALIB_HTU_T, "HTU TEMP", DisplayUI::HandleDrawCalib,     DisplayUI::HandleUpCalib, DisplayUI::HandleDownCalib, DisplayUI::HandleNextItem, DisplayUI::HandleExitSubmenu },
  { MenuItem::CALIB_HTU_H, "HTU HUM",  DisplayUI::HandleDrawCalib,     DisplayUI::HandleUpCalib, DisplayUI::HandleDownCalib, DisplayUI::HandleNextItem, DisplayUI::HandleExitSubmenu },
  { MenuItem::CALIB_DS_T,  "DS TEMP",  DisplayUI::HandleDrawCalib,     DisplayUI::HandleUpCalib, DisplayUI::HandleDownCalib, DisplayUI::HandleNextItem, DisplayUI::HandleExitSubmenu }
};

static const MenuRootDef MENU_TABLE[] = {
  { MenuRoot::HOME,    "HOME",    nullptr,       0, DisplayUI::HandlePrevRoot, DisplayUI::HandleNextRoot, DisplayUI::HandleNextRoot, nullptr },
  { MenuRoot::STATUS,  "STATUS",  STATUS_ITEMS,  2, DisplayUI::HandlePrevRoot, DisplayUI::HandleNextRoot, DisplayUI::HandleNextRoot, DisplayUI::HandleEnterSubmenu },
  { MenuRoot::TARGETS, "TARGETS", TARGET_ITEMS,  2, DisplayUI::HandlePrevRoot, DisplayUI::HandleNextRoot, DisplayUI::HandleNextRoot, DisplayUI::HandleEnterSubmenu },
  { MenuRoot::MANUAL,  "MANUAL",  MANUAL_ITEMS,  2, DisplayUI::HandlePrevRoot, DisplayUI::HandleNextRoot, DisplayUI::HandleNextRoot, DisplayUI::HandleEnterSubmenu },
  { MenuRoot::STATS,   "STATS",   STATS_ITEMS,   2, DisplayUI::HandlePrevRoot, DisplayUI::HandleNextRoot, DisplayUI::HandleNextRoot, DisplayUI::HandleEnterSubmenu },
  { MenuRoot::ERRORS,  "ERRORS",  ERROR_ITEMS,   1, DisplayUI::HandlePrevRoot, DisplayUI::HandleNextRoot, DisplayUI::HandleNextRoot, DisplayUI::HandleEnterSubmenu },
  { MenuRoot::SERVICE, "SERVICE", SERVICE_ITEMS, 5, DisplayUI::HandlePrevRoot, DisplayUI::HandleNextRoot, DisplayUI::HandleNextRoot, DisplayUI::HandleEnterSubmenu }
};

static const uint8_t MENU_TABLE_SIZE = sizeof(MENU_TABLE) / sizeof(MENU_TABLE[0]);

void DisplayUI::HandlePrevRoot(DisplayUI* ui) {
  ui->root_index_ = (ui->root_index_ - 1 + MENU_TABLE_SIZE) % MENU_TABLE_SIZE;
  ui->item_index_ = 0;
  ui->in_submenu_ = false;
}
void DisplayUI::HandleNextRoot(DisplayUI* ui) {
  ui->root_index_ = (ui->root_index_ + 1) % MENU_TABLE_SIZE;
  ui->item_index_ = 0;
  ui->in_submenu_ = false;
}

// Конструктор: адрес 0x27 и размер 16x2
DisplayUI::DisplayUI(Controller* c, SensorManager* s, TimeManager* t)
    : lcd_(0x27, 16, 2),
      controller_(c),
      sensors_(s),
      rtc_(t),
      root_index_(0),
      item_index_(0),
      in_submenu_(false),
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

const MenuRootDef* DisplayUI::GetCurrentRootDef() const {
  return &MENU_TABLE[root_index_];
}

const MenuItemDef* DisplayUI::GetCurrentItemDef() const {
  const MenuRootDef* root = GetCurrentRootDef();
  if (root && root->items && item_index_ < root->item_count) {
    return &root->items[item_index_];
  }
  return nullptr;
}

void DisplayUI::Update() {
  HandleButtons();    // Опрос кнопок
  UpdateBacklight();  // Управление светом

  static unsigned long last_draw = 0;
  unsigned long interval = (root_index_ == 0 && !in_submenu_) ? 500 : 1000;

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
      if (!in_submenu_) {
        const MenuRootDef* root = GetCurrentRootDef();
        if (root) {
          if (press_duration < 600) {
            if (root->on_menu) root->on_menu(this);
          } else {
            if (root->on_long_menu) root->on_long_menu(this);
          }
        }
      } else {
        const MenuItemDef* item = GetCurrentItemDef();
        if (item) {
          if (press_duration < 600) {
            if (item->on_menu) item->on_menu(this);
          } else {
            if (item->on_long_menu) item->on_long_menu(this);
          }
        }
      }
      menu_btn_pressed_ = false;
      needs_redraw_ = true;
    }
  }

  // Логика кнопок изменения значений (UP/DOWN)
  if ((up || down) && !menu_btn_pressed_) {
    if (millis() - last_btn_action_ >= 150) {
      last_btn_action_ = millis();
      if (!in_submenu_) {
        const MenuRootDef* root = GetCurrentRootDef();
        if (root) {
          if (up && root->on_up) root->on_up(this);
          if (down && root->on_down) root->on_down(this);
        }
      } else {
        const MenuItemDef* item = GetCurrentItemDef();
        if (item) {
          if (up && item->on_up) item->on_up(this);
          if (down && item->on_down) item->on_down(this);
        }
      }
      needs_redraw_ = true;
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
  if (root_index_ == 0) { // HOME
    DrawHomeScreen();
  } else if (!in_submenu_) {
    DrawRootPage();
  } else {
    const MenuItemDef* item = GetCurrentItemDef();
    if (item && item->draw) {
      item->draw(this);
    }
  }
}

void DisplayUI::DrawRootPage() {
  const MenuRootDef* root = GetCurrentRootDef();
  DrawHeader(root ? root->label : "", root_index_ - 1, MENU_TABLE_SIZE - 1);

  screen_.SetPos(1, 0);
  screen_.print(F("  MENU ENTER"));
}

void DisplayUI::DrawHeader(const char* label, uint8_t index, uint8_t count) {
  screen_.SetPos(0, 0);
  screen_.print(label);
  screen_.print(F(" "));
  screen_.print(index + 1);
  screen_.print(F("/"));
  screen_.print(count);
}

void DisplayUI::DrawHeader(const __FlashStringHelper* label, uint8_t index, uint8_t count) {
  screen_.SetPos(0, 0);
  screen_.print(label);
  screen_.print(F(" "));
  screen_.print(index + 1);
  screen_.print(F("/"));
  screen_.print(count);
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
  const MenuRootDef* root = GetCurrentRootDef();
  DrawHeader(F("STATUS"), item_index_, root->item_count);

  SensorData in = sensors_->GetInside();
  screen_.SetPos(1, 0);
  screen_.print(F("IN "));
  screen_.print(in.temp, 1);
  screen_.print(F("C "));
  screen_.print(in.rh, 0);
  screen_.print(F("%"));
}

void DisplayUI::DrawStatusOut() {
  const MenuRootDef* root = GetCurrentRootDef();
  DrawHeader(F("STATUS"), item_index_, root->item_count);

  SensorData out = sensors_->GetOutside();
  screen_.SetPos(1, 0);
  screen_.print(F("OUT "));
  screen_.print(out.temp, 1);
  screen_.print(F("C "));
  screen_.print(out.rh, 0);
  screen_.print(F("%"));
}

void DisplayUI::DrawTargets() {
  const MenuRootDef* root = GetCurrentRootDef();
  DrawHeader(F("TARGETS"), item_index_, root->item_count);

  screen_.SetPos(1, 0);
  if (item_index_ == 0) // TARGET_TEMP
    screen_.print(F(">"));
  else
    screen_.print(F(" "));
  screen_.print(F("T:"));
  screen_.print(controller_->GetTargetTemp(), 1);

  screen_.print(F(" "));
  if (item_index_ == 1) // TARGET_HUM
    screen_.print(F(">"));
  else
    screen_.print(F(" "));
  screen_.print(F("H:"));
  screen_.print(controller_->GetTargetRh(), 0);
  screen_.print(F("%"));
}

void DisplayUI::DrawManualModes() {
  const MenuRootDef* root = GetCurrentRootDef();
  DrawHeader(F("MANUAL"), item_index_, root->item_count);

  screen_.SetPos(1, 0);
  if (item_index_ == 0) // MANUAL_FAN
    screen_.print(F(">"));
  else
    screen_.print(F(" "));
  screen_.print(F("FAN   "));

  if (item_index_ == 1) // MANUAL_OZONE
    screen_.print(F(">"));
  else
    screen_.print(F(" "));
  screen_.print(F("OZONE"));
}

void DisplayUI::DrawCalibPage(const char* label, float value, bool is_temp) {
  const MenuRootDef* root = GetCurrentRootDef();
  DrawHeader(F("SERVICE"), item_index_, root->item_count);

  screen_.SetPos(1, 0);
  screen_.print(label);
  screen_.print(F(" "));
  if (value >= 0) screen_.print(F("+"));
  screen_.print(value, 1);
  screen_.print(is_temp ? F("C") : F("%"));
}

void DisplayUI::DrawStats() {
  const MenuRootDef* root = GetCurrentRootDef();
  DrawHeader(F("STATS"), item_index_, root->item_count);

  screen_.SetPos(1, 0);
  if (item_index_ == 0) { // STATS_VIEW
    SystemStatistics s = controller_->GetStats();
    screen_.print(F("U:"));
    screen_.print(s.uptimeMinutes / 60);
    screen_.print(F(" F:"));
    screen_.print(s.fanMinutes / 60);
    screen_.print(F(" O3:"));
    screen_.print(s.ozoneMinutes / 60);
  } else if (item_index_ == 1) { // STATS_RESET
    screen_.print(F("MENU CONFIRM"));
  }
}

void DisplayUI::DrawErrorLog() {
  const MenuRootDef* root = GetCurrentRootDef();
  DrawHeader(F("ERRORS"), item_index_, root->item_count);

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
