#include "DisplayUI.h"
#include "config.h"

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
          if (current_root_ == MenuRoot::MANUAL) {
            if (current_item_ == MenuItem::MANUAL_FAN) {
              controller_->StartManualFan(30);
              temp_message_ = "FAN STARTED";
              message_timer_ = millis();
            } else if (current_item_ == MenuItem::MANUAL_OZONE) {
              controller_->StartManualOzone(15);
              temp_message_ = "OZONE STARTED";
              message_timer_ = millis();
            }
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
        CalibrationData c = controller_->GetCalibration();
        if (up) {
          switch (current_item_) {
            case MenuItem::TARGET_TEMP:
              controller_->SetTargetTemp(controller_->GetTargetTemp() + 0.1f);
              break;
            case MenuItem::TARGET_HUM:
              controller_->SetTargetRh(controller_->GetTargetRh() + 1.0f);
              if (controller_->GetTargetRh() > 100.0f)
                controller_->SetTargetRh(100.0f);
              break;
            case MenuItem::MANUAL_FAN:
            case MenuItem::MANUAL_OZONE:
              current_item_ = MenuItem::MANUAL_FAN;
              break;
            case MenuItem::CALIB_BME_T:
              c.bmeTempOffset += 0.1f;
              if (c.bmeTempOffset > 5.0f) c.bmeTempOffset = 5.0f;
              controller_->SetCalibration(c);
              break;
            case MenuItem::CALIB_BME_H:
              c.bmeHumOffset += 0.1f;
              if (c.bmeHumOffset > 5.0f) c.bmeHumOffset = 5.0f;
              controller_->SetCalibration(c);
              break;
            case MenuItem::CALIB_HTU_T:
              c.htuTempOffset += 0.1f;
              if (c.htuTempOffset > 5.0f) c.htuTempOffset = 5.0f;
              controller_->SetCalibration(c);
              break;
            case MenuItem::CALIB_HTU_H:
              c.htuHumOffset += 0.1f;
              if (c.htuHumOffset > 5.0f) c.htuHumOffset = 5.0f;
              controller_->SetCalibration(c);
              break;
            case MenuItem::CALIB_DS_T:
              c.dsTempOffset += 0.1f;
              if (c.dsTempOffset > 5.0f) c.dsTempOffset = 5.0f;
              controller_->SetCalibration(c);
              break;
            case MenuItem::ERROR_VIEW:
              controller_->ResetError();
              break;
            default:
              break;
          }
        } else if (down) {
          switch (current_item_) {
            case MenuItem::TARGET_TEMP:
              controller_->SetTargetTemp(controller_->GetTargetTemp() - 0.1f);
              break;
            case MenuItem::TARGET_HUM:
              controller_->SetTargetRh(controller_->GetTargetRh() - 1.0f);
              if (controller_->GetTargetRh() < 0.0f)
                controller_->SetTargetRh(0.0f);
              break;
            case MenuItem::MANUAL_FAN:
            case MenuItem::MANUAL_OZONE:
              current_item_ = MenuItem::MANUAL_OZONE;
              break;
            case MenuItem::CALIB_BME_T:
              c.bmeTempOffset -= 0.1f;
              if (c.bmeTempOffset < -5.0f) c.bmeTempOffset = -5.0f;
              controller_->SetCalibration(c);
              break;
            case MenuItem::CALIB_BME_H:
              c.bmeHumOffset -= 0.1f;
              if (c.bmeHumOffset < -5.0f) c.bmeHumOffset = -5.0f;
              controller_->SetCalibration(c);
              break;
            case MenuItem::CALIB_HTU_T:
              c.htuTempOffset -= 0.1f;
              if (c.htuTempOffset < -5.0f) c.htuTempOffset = -5.0f;
              controller_->SetCalibration(c);
              break;
            case MenuItem::CALIB_HTU_H:
              c.htuHumOffset -= 0.1f;
              if (c.htuHumOffset < -5.0f) c.htuHumOffset = -5.0f;
              controller_->SetCalibration(c);
              break;
            case MenuItem::CALIB_DS_T:
              c.dsTempOffset -= 0.1f;
              if (c.dsTempOffset < -5.0f) c.dsTempOffset = -5.0f;
              controller_->SetCalibration(c);
              break;
            default:
              break;
          }
        }
      }
    }
  }
}

void DisplayUI::NextRoot() {
  int next = (int)current_root_ + 1;
  if (next > (int)MenuRoot::SERVICE) next = 0;
  current_root_ = (MenuRoot)next;
  in_submenu_ = false;
  SetDefaultItemForRoot();
  needs_redraw_ = true;
}

void DisplayUI::NextItem() {
  switch (current_root_) {
    case MenuRoot::STATUS:
      current_item_ = (current_item_ == MenuItem::STATUS_IN) ? MenuItem::STATUS_OUT : MenuItem::STATUS_IN;
      break;
    case MenuRoot::TARGETS:
      current_item_ = (current_item_ == MenuItem::TARGET_TEMP) ? MenuItem::TARGET_HUM : MenuItem::TARGET_TEMP;
      break;
    case MenuRoot::MANUAL:
      current_item_ = (current_item_ == MenuItem::MANUAL_FAN) ? MenuItem::MANUAL_OZONE : MenuItem::MANUAL_FAN;
      break;
    case MenuRoot::STATS:
      current_item_ = (current_item_ == MenuItem::STATS_VIEW) ? MenuItem::STATS_RESET : MenuItem::STATS_VIEW;
      break;
    case MenuRoot::SERVICE:
      {
        int next = (int)current_item_ + 1;
        if (next > (int)MenuItem::CALIB_DS_T) next = (int)MenuItem::CALIB_BME_T;
        current_item_ = (MenuItem)next;
      }
      break;
    default:
      break;
  }
  UpdateSubmenuIndex();
  needs_redraw_ = true;
}

void DisplayUI::PrevItem() {
  switch (current_root_) {
    case MenuRoot::STATUS:
      current_item_ = (current_item_ == MenuItem::STATUS_IN) ? MenuItem::STATUS_OUT : MenuItem::STATUS_IN;
      break;
    case MenuRoot::TARGETS:
      current_item_ = (current_item_ == MenuItem::TARGET_TEMP) ? MenuItem::TARGET_HUM : MenuItem::TARGET_TEMP;
      break;
    case MenuRoot::MANUAL:
      current_item_ = (current_item_ == MenuItem::MANUAL_FAN) ? MenuItem::MANUAL_OZONE : MenuItem::MANUAL_FAN;
      break;
    case MenuRoot::STATS:
      current_item_ = (current_item_ == MenuItem::STATS_VIEW) ? MenuItem::STATS_RESET : MenuItem::STATS_VIEW;
      break;
    case MenuRoot::SERVICE:
      {
        int next = (int)current_item_ - 1;
        if (next < (int)MenuItem::CALIB_BME_T) next = (int)MenuItem::CALIB_DS_T;
        current_item_ = (MenuItem)next;
      }
      break;
    default:
      break;
  }
  UpdateSubmenuIndex();
  needs_redraw_ = true;
}

void DisplayUI::SetDefaultItemForRoot() {
  switch (current_root_) {
    case MenuRoot::HOME:    current_item_ = MenuItem::NONE; break;
    case MenuRoot::STATUS:  current_item_ = MenuItem::STATUS_IN; break;
    case MenuRoot::TARGETS: current_item_ = MenuItem::TARGET_TEMP; break;
    case MenuRoot::MANUAL:  current_item_ = MenuItem::MANUAL_FAN; break;
    case MenuRoot::STATS:   current_item_ = MenuItem::STATS_VIEW; break;
    case MenuRoot::ERRORS:  current_item_ = MenuItem::ERROR_VIEW; break;
    case MenuRoot::SERVICE: current_item_ = MenuItem::CALIB_BME_T; break;
  }
  UpdateSubmenuIndex();
  needs_redraw_ = true;
}

void DisplayUI::UpdateSubmenuIndex() {
  switch (current_root_) {
    case MenuRoot::STATUS:
      submenu_count_ = 2;
      submenu_index_ = (current_item_ == MenuItem::STATUS_IN) ? 1 : 2;
      break;
    case MenuRoot::TARGETS:
      submenu_count_ = 2;
      submenu_index_ = (current_item_ == MenuItem::TARGET_TEMP) ? 1 : 2;
      break;
    case MenuRoot::MANUAL:
      submenu_count_ = 2;
      submenu_index_ = (current_item_ == MenuItem::MANUAL_FAN) ? 1 : 2;
      break;
    case MenuRoot::STATS:
      submenu_count_ = 2;
      submenu_index_ = (current_item_ == MenuItem::STATS_VIEW) ? 1 : 2;
      break;
    case MenuRoot::ERRORS:
      submenu_count_ = 1;
      submenu_index_ = 1;
      break;
    case MenuRoot::SERVICE:
      submenu_count_ = 5;
      submenu_index_ = (int)current_item_ - (int)MenuItem::CALIB_BME_T + 1;
      break;
    default:
      submenu_count_ = 0;
      submenu_index_ = 0;
      break;
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

  screen_.SetPos(0, 0);
  switch (current_root_) {
    case MenuRoot::STATUS:  screen_.print(F("> STATUS")); break;
    case MenuRoot::TARGETS: screen_.print(F("> TARGETS")); break;
    case MenuRoot::MANUAL:  screen_.print(F("> MANUAL")); break;
    case MenuRoot::STATS:   screen_.print(F("> STATS")); break;
    case MenuRoot::ERRORS:  screen_.print(F("> ERRORS")); break;
    case MenuRoot::SERVICE: screen_.print(F("> SERVICE")); break;
    default: break;
  }
  screen_.SetPos(1, 0);
  screen_.print(F("  MENU ENTER"));
}

void DisplayUI::DrawSubPage() {
  CalibrationData c = controller_->GetCalibration();
  switch (current_item_) {
    case MenuItem::STATUS_IN:
      DrawStatusIn();
      break;
    case MenuItem::STATUS_OUT:
      DrawStatusOut();
      break;
    case MenuItem::TARGET_TEMP:
    case MenuItem::TARGET_HUM:
      DrawTargets();
      break;
    case MenuItem::MANUAL_FAN:
    case MenuItem::MANUAL_OZONE:
      DrawManualModes();
      break;
    case MenuItem::STATS_VIEW:
    case MenuItem::STATS_RESET:
      DrawStats();
      break;
    case MenuItem::ERROR_VIEW:
      DrawErrorLog();
      break;
    case MenuItem::CALIB_BME_T:
      DrawCalibPage("BME TEMP", c.bmeTempOffset, true);
      break;
    case MenuItem::CALIB_BME_H:
      DrawCalibPage("BME HUM", c.bmeHumOffset, false);
      break;
    case MenuItem::CALIB_HTU_T:
      DrawCalibPage("HTU TEMP", c.htuTempOffset, true);
      break;
    case MenuItem::CALIB_HTU_H:
      DrawCalibPage("HTU HUM", c.htuHumOffset, false);
      break;
    case MenuItem::CALIB_DS_T:
      DrawCalibPage("DS TEMP", c.dsTempOffset, true);
      break;
    default:
      break;
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
