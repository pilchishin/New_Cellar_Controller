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
      last_btn_check_(0),
      last_btn_action_(0),
      menu_btn_pressed_(false),
      message_timer_(0),
      temp_message_(nullptr),
      backlight_on_(true) {}

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

  // Обновляем экран раз в 500мс, чтобы не мерцал
  static unsigned long last_draw = 0;
  if (millis() - last_draw >= 500) {
    last_draw = millis();

    // Если отображается временное сообщение, ничего другого не рисуем
    if (temp_message_ != nullptr && millis() - message_timer_ < 2000) {
      lcd_.setCursor(0, 0);
      lcd_.print(F("                "));
      lcd_.setCursor(0, 0);
      lcd_.print(temp_message_);
      lcd_.setCursor(0, 1);
      lcd_.print(F("                "));
    } else {
      if (temp_message_ != nullptr) {
        temp_message_ = nullptr;  // Сброс сообщения по истечении времени
        lcd_.clear();
      }
      DrawPage();
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
          NextItem();
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
      lcd_.clear();
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
              controller_->StartManualFan(30);
              temp_message_ = "FAN STARTED";
              message_timer_ = millis();
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
              controller_->StartManualOzone(15);
              temp_message_ = "OZONE STARTED";
              message_timer_ = millis();
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

  lcd_.setCursor(0, 0);
  switch (current_root_) {
    case MenuRoot::STATUS:  lcd_.print(F("> STATUS        ")); break;
    case MenuRoot::TARGETS: lcd_.print(F("> TARGETS       ")); break;
    case MenuRoot::MANUAL:  lcd_.print(F("> MANUAL        ")); break;
    case MenuRoot::STATS:   lcd_.print(F("> STATS         ")); break;
    case MenuRoot::ERRORS:  lcd_.print(F("> ERRORS        ")); break;
    case MenuRoot::SERVICE: lcd_.print(F("> SERVICE       ")); break;
    default: break;
  }
  lcd_.setCursor(0, 1);
  lcd_.print(F("  MENU ENTER    "));
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
      DrawSetTemp();
      break;
    case MenuItem::TARGET_HUM:
      DrawSetHum();
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
  lcd_.setCursor(0, 0);
  lcd_.print(F("IN "));
  lcd_.print(in.temp, 1);
  lcd_.print(F("C "));
  lcd_.print(in.rh, 0);
  lcd_.print(F("%     "));

  lcd_.setCursor(15, 0);
  if (rm->GetOzoneState())
    lcd_.print(F("O"));
  else if (rm->GetFanState())
    lcd_.print(F("F"));
  else
    lcd_.print(F(" "));

  // Строка 2: OUT temp humidity режим системы
  lcd_.setCursor(0, 1);
  lcd_.print(F("OUT "));
  lcd_.print(out.temp, 1);
  lcd_.print(F("C "));
  lcd_.print(out.rh, 0);
  lcd_.print(F("%    "));

  lcd_.setCursor(15, 1);
  SystemState state = controller_->GetState();
  if (state == SystemState::kErrorState)
    lcd_.print(F("E"));
  else if (state == SystemState::kAutoClimate ||
           state == SystemState::kOzoneStart ||
           state == SystemState::kOzoneActive ||
           state == SystemState::kOzoneHold ||
           state == SystemState::kOzoneVent)
    lcd_.print(F("A"));
  else
    lcd_.print(F("M"));
}

void DisplayUI::DrawStatusIn() {
  SensorData in = sensors_->GetInside();
  lcd_.setCursor(0, 0);
  lcd_.print(F("IN "));
  lcd_.print(in.temp, 1);
  lcd_.print(F("C "));
  lcd_.print(in.rh, 0);
  lcd_.print(F("%         "));

  lcd_.setCursor(0, 1);
  lcd_.print(F("DP "));
  lcd_.print(in.dewpoint, 1);
  lcd_.print(F(" AH "));
  lcd_.print(in.ah, 1);
  lcd_.print(F("        "));
}

void DisplayUI::DrawStatusOut() {
  SensorData out = sensors_->GetOutside();
  lcd_.setCursor(0, 0);
  lcd_.print(F("OUT "));
  lcd_.print(out.temp, 1);
  lcd_.print(F("C "));
  lcd_.print(out.rh, 0);
  lcd_.print(F("%        "));

  lcd_.setCursor(0, 1);
  lcd_.print(F("AH "));
  lcd_.print(out.ah, 2);
  lcd_.print(F("             "));
}

void DisplayUI::DrawSetTemp() {
  lcd_.setCursor(0, 0);
  lcd_.print(F("SET TARGET TEMP"));
  lcd_.setCursor(0, 1);
  lcd_.print(F("TEMP: "));
  lcd_.print(controller_->GetTargetTemp(), 1);
  lcd_.print(F("C"));
}

void DisplayUI::DrawSetHum() {
  lcd_.setCursor(0, 0);
  lcd_.print(F("SET TARGET HUM"));
  lcd_.setCursor(0, 1);
  lcd_.print(F("HUM:  "));
  lcd_.print(controller_->GetTargetRh(), 0);
  lcd_.print(F("%"));
}

void DisplayUI::DrawManualModes() {
  lcd_.setCursor(0, 0);
  lcd_.print(F("MANUAL START:"));
  lcd_.setCursor(0, 1);
  lcd_.print(F("UP:FAN  DN:OZONE"));
}

void DisplayUI::DrawCalibPage(const char* label, float value, bool is_temp) {
  lcd_.setCursor(0, 0);
  lcd_.print(F("CAL:"));
  lcd_.print(label);
  lcd_.print(F("        "));  // Очистка остатка строки
  lcd_.setCursor(0, 1);
  lcd_.print(F("OFFS:"));
  if (value >= 0) lcd_.print(F("+"));
  lcd_.print(value, 1);
  lcd_.print(is_temp ? F("C") : F("% "));
  lcd_.print(F("      "));  // Очистка
}

void DisplayUI::DrawStats() {
  SystemStatistics s = controller_->GetStats();
  lcd_.setCursor(0, 0);
  lcd_.print(F("U:"));
  lcd_.print(s.uptimeMinutes / 60);
  lcd_.print(F("h "));
  lcd_.print(F("F:"));
  lcd_.print(s.fanMinutes / 60);
  lcd_.print(F("h    "));

  lcd_.setCursor(0, 1);
  lcd_.print(F("O3:"));
  lcd_.print(s.ozoneMinutes / 60);
  lcd_.print(F("h "));
  lcd_.print(F("M:RESET "));
}

void DisplayUI::DrawErrorLog() {
  lcd_.setCursor(0, 0);
  lcd_.print(F("LAST ERROR:     "));
  lcd_.setCursor(0, 1);
  ErrorCode err = controller_->GetError();
  if (err == ErrorCode::kNone) {
    lcd_.print(F("SYSTEM OK       "));
  } else {
    lcd_.print(ErrorToString(err));
    lcd_.setCursor(10, 1);
    lcd_.print(F("UP:RST"));
  }
}
