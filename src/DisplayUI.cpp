#include "DisplayUI.h"
#include "MenuActions.h"
#include "config.h"


// --- Определения таблиц меню в PROGMEM ---

static const char lbl_status_in[] PROGMEM = "STATUS_IN";
static const char lbl_status_out[] PROGMEM = "STATUS_OUT";
static const MenuItemDef STATUS_ITEMS[] PROGMEM = {
  { MenuItemID::kStatusIn,  lbl_status_in,  MenuItemType::kView, 0, ActionID::kNavPrev, ActionID::kNavNext, ActionID::kNavNext, ActionID::kExitSubmenu },
  { MenuItemID::kStatusOut, lbl_status_out, MenuItemType::kView, 1, ActionID::kNavPrev, ActionID::kNavNext, ActionID::kNavNext, ActionID::kExitSubmenu }
};

// --- Определения страниц редактирования значений ---

static const char unit_c[] PROGMEM = "C";
static const char unit_pct[] PROGMEM = "%";

const ValuePageDef VALUE_PAGES[] PROGMEM = {
  { MenuItemID::kTargetTemp,   ValueID::kTargetTemp,   kTempCriticalMin, 15.0f, 0.1f, unit_c,   1 },
  { MenuItemID::kTargetHum,    ValueID::kTargetHum,    0.0f, 100.0f, 1.0f, unit_pct, 0 },
  { MenuItemID::kCalibBmeTemp, ValueID::kCalibBmeTemp, -5.0f,  5.0f, 0.1f, unit_c,   1 },
  { MenuItemID::kCalibBmeHum,  ValueID::kCalibBmeHum,  -5.0f,  5.0f, 0.1f, unit_pct, 1 },
  { MenuItemID::kCalibHtuTemp, ValueID::kCalibHtuTemp, -5.0f,  5.0f, 0.1f, unit_c,   1 },
  { MenuItemID::kCalibHtuHum,  ValueID::kCalibHtuHum,  -5.0f,  5.0f, 0.1f, unit_pct, 1 },
  { MenuItemID::kCalibDsTemp,  ValueID::kCalibDsTemp,  -5.0f,  5.0f, 0.1f, unit_c,   1 }
};

static const char lbl_target_t[] PROGMEM = "TEMP";
static const char lbl_target_h[] PROGMEM = "HUM";
static const MenuItemDef TARGET_ITEMS[] PROGMEM = {
  { MenuItemID::kTargetTemp, lbl_target_t, MenuItemType::kValue, 0, ActionID::kValueInc, ActionID::kValueDec, ActionID::kNavNext, ActionID::kExitSubmenu },
  { MenuItemID::kTargetHum,  lbl_target_h, MenuItemType::kValue, 1, ActionID::kValueInc, ActionID::kValueDec, ActionID::kNavNext, ActionID::kExitSubmenu }
};

static const char lbl_man_fan[] PROGMEM = "MANUAL_FAN";
static const char lbl_man_o3[] PROGMEM = "MANUAL_OZONE";
static const MenuItemDef MANUAL_ITEMS[] PROGMEM = {
  { MenuItemID::kManualFan,   lbl_man_fan,   MenuItemType::kAction, 0, ActionID::kNavPrev, ActionID::kNavNext, ActionID::kManualStart, ActionID::kExitSubmenu },
  { MenuItemID::kManualOzone, lbl_man_o3,    MenuItemType::kAction, 0, ActionID::kNavPrev, ActionID::kNavNext, ActionID::kManualStart, ActionID::kExitSubmenu }
};

static const char lbl_stats_v[] PROGMEM = "STATS_VIEW";
static const char lbl_stats_r[] PROGMEM = "STATS_RESET";
static const MenuItemDef STATS_ITEMS[] PROGMEM = {
  { MenuItemID::kStatsView,  lbl_stats_v, MenuItemType::kStats, 0, ActionID::kNavPrev,    ActionID::kNavNext, ActionID::kNavNext,     ActionID::kExitSubmenu },
  { MenuItemID::kStatsReset, lbl_stats_r, MenuItemType::kStats, 0, ActionID::kNavPrev,    ActionID::kNavNext, ActionID::kNavNext,     ActionID::kStatsReset }
};

static const char lbl_err_v[] PROGMEM = "ERROR_VIEW";
static const MenuItemDef ERROR_ITEMS[] PROGMEM = {
  { MenuItemID::kErrorView, lbl_err_v, MenuItemType::kView, 0, ActionID::kErrorReset, ActionID::kNone,    ActionID::kNavNext,     ActionID::kExitSubmenu }
};

static const char lbl_bme_t[] PROGMEM = "BME T";
static const char lbl_bme_h[] PROGMEM = "BME H";
static const char lbl_htu_t[] PROGMEM = "HTU T";
static const char lbl_htu_h[] PROGMEM = "HTU H";
static const char lbl_ds_t[] PROGMEM = "DS T";

static const MenuItemDef SERVICE_ITEMS[] PROGMEM = {
  { MenuItemID::kCalibBmeTemp, lbl_bme_t, MenuItemType::kValue, 2, ActionID::kValueInc, ActionID::kValueDec, ActionID::kNavNext, ActionID::kExitSubmenu },
  { MenuItemID::kCalibBmeHum,  lbl_bme_h, MenuItemType::kValue, 3, ActionID::kValueInc, ActionID::kValueDec, ActionID::kNavNext, ActionID::kExitSubmenu },
  { MenuItemID::kCalibHtuTemp, lbl_htu_t, MenuItemType::kValue, 4, ActionID::kValueInc, ActionID::kValueDec, ActionID::kNavNext, ActionID::kExitSubmenu },
  { MenuItemID::kCalibHtuHum,  lbl_htu_h, MenuItemType::kValue, 5, ActionID::kValueInc, ActionID::kValueDec, ActionID::kNavNext, ActionID::kExitSubmenu },
  { MenuItemID::kCalibDsTemp,  lbl_ds_t,  MenuItemType::kValue, 6, ActionID::kValueInc, ActionID::kValueDec, ActionID::kNavNext, ActionID::kExitSubmenu }
};

// Текстовые метки корневых разделов
static const char root_home[] PROGMEM = "HOME";
static const char root_status[] PROGMEM = "STATUS";
static const char root_targets[] PROGMEM = "TARGETS";
static const char root_manual[] PROGMEM = "MANUAL";
static const char root_stats[] PROGMEM = "STATS";
static const char root_errors[] PROGMEM = "ERRORS";
static const char root_service[] PROGMEM = "SERVICE";

/**
 * @brief Главная таблица разделов меню.
 * Хранится во Flash-памяти для экономии RAM.
 */
static const MenuRootDef MENU_TABLE[] PROGMEM = {
  { root_home,    nullptr,       0, ActionID::kNavPrevRoot, ActionID::kNavNextRoot, ActionID::kNavNextRoot, ActionID::kNone,          500 },
  { root_status,  STATUS_ITEMS,  2, ActionID::kNavPrevRoot, ActionID::kNavNextRoot, ActionID::kNavNextRoot, ActionID::kEnterSubmenu, 1000 },
  { root_targets, TARGET_ITEMS,  2, ActionID::kNavPrevRoot, ActionID::kNavNextRoot, ActionID::kNavNextRoot, ActionID::kEnterSubmenu, 1000 },
  { root_manual,  MANUAL_ITEMS,  2, ActionID::kNavPrevRoot, ActionID::kNavNextRoot, ActionID::kNavNextRoot, ActionID::kEnterSubmenu, 1000 },
  { root_stats,   STATS_ITEMS,   2, ActionID::kNavPrevRoot, ActionID::kNavNextRoot, ActionID::kNavNextRoot, ActionID::kEnterSubmenu, 1000 },
  { root_errors,  ERROR_ITEMS,   1, ActionID::kNavPrevRoot, ActionID::kNavNextRoot, ActionID::kNavNextRoot, ActionID::kEnterSubmenu, 1000 },
  { root_service, SERVICE_ITEMS, 5, ActionID::kNavPrevRoot, ActionID::kNavNextRoot, ActionID::kNavNextRoot, ActionID::kEnterSubmenu, 1000 }
};

static const uint8_t MENU_TABLE_SIZE = sizeof(MENU_TABLE) / sizeof(MENU_TABLE[0]);

uint8_t DisplayUI::GetMenuTableSize() const { return MENU_TABLE_SIZE; }


/**
 * @brief Конструктор UI.
 * Инициализирует базовые параметры и очищает кэш строк.
 */
DisplayUI::DisplayUI(Controller* c, SensorManager* s, TimeManager* t)
    : lcd_(0x27, 16, 2),
      controller_(c),
      sensors_(s),
      rtc_(t),
      nav_(),
      buttons_(BT_UP, BT_DOWN, BT_MENU),
      message_timer_(0),
      temp_message_(nullptr),
      last_activity_time_(0),
      backlight_on_(true),
      needs_redraw_(true) {
  memset(last_lines_, 0, sizeof(last_lines_));
}

/**
 * @brief Первичная инициализация LCD и пинов кнопок.
 */
void DisplayUI::Init() {
  lcd_.init();
  lcd_.backlight();
  buttons_.Init();
  last_activity_time_ = millis();
}

/**
 * @brief Восстановление работы LCD после возможных сбоев I2C-шины.
 */
void DisplayUI::Reinit() {
  lcd_.init();
  if (backlight_on_)
    lcd_.backlight();
  else
    lcd_.noBacklight();
}

// Статические буферы для временного хранения структур, прочитанных из Flash
MenuRootDef DisplayUI::current_root_buf_;
MenuItemDef DisplayUI::current_item_buf_;

/**
 * @brief Чтение определения текущего корневого раздела из PROGMEM в RAM-буфер.
 */
const MenuRootDef* DisplayUI::GetCurrentRootDef() const {
  memcpy_P(&current_root_buf_, &MENU_TABLE[nav_.GetRootIndex()], sizeof(MenuRootDef));
  return &current_root_buf_;
}

/**
 * @brief Чтение определения текущего элемента подменю из PROGMEM в RAM-буфер.
 * Использует pgm_read_ptr для получения адреса массива элементов.
 */
const MenuItemDef* DisplayUI::GetCurrentItemDef() const {
  // Согласно лучшим практикам AVR, сначала читаем указатель из PROGMEM
  uint8_t root_idx = nav_.GetRootIndex();
  uint8_t item_idx = nav_.GetItemIndex();
  const MenuItemDef* items_ptr = (const MenuItemDef*)pgm_read_ptr(&MENU_TABLE[root_idx].items);
  uint8_t count = pgm_read_byte(&MENU_TABLE[root_idx].item_count);

  if (items_ptr && item_idx < count) {
    // Затем копируем всю структуру элемента в RAM
    memcpy_P(&current_item_buf_, &items_ptr[item_idx], sizeof(MenuItemDef));
    return &current_item_buf_;
  }
  return nullptr;
}

/**
 * @brief Основной цикл обновления интерфейса.
 * Обрабатывает кнопки, подсветку и перерисовку по таймеру или флагу.
 */
void DisplayUI::Update() {
  model_.Sync(controller_, sensors_);
  HandleButtons();    // Опрос кнопок
  UpdateBacklight();  // Управление тайм-аутом света

  static unsigned long last_draw = 0;
  const MenuRootDef* root = GetCurrentRootDef();
  unsigned long interval = root ? root->refresh_interval_ms : 1000;

  // Перерисовка по таймеру или по событию (нажатие кнопки)
  if (needs_redraw_ || (millis() - last_draw >= interval)) {
    last_draw = millis();
    needs_redraw_ = false;

    screen_.Clear();

    // Приоритет вывода - временное системное сообщение
    if (temp_message_ != nullptr && millis() - message_timer_ < 2000) {
      screen_.SetPos(0, 0);
      screen_.print(temp_message_);
      screen_.SetPos(1, 0);
      screen_.print(F("                "));
    } else {
      if (temp_message_ != nullptr) {
        temp_message_ = nullptr;
      }
      DrawPage(); // Наполнение буфера содержимым страницы
    }

    Flush(); // Физический вывод изменений на LCD
  }
}

/**
 * @brief Оптимизированный вывод на LCD.
 * Печатает строку только если её содержимое изменилось по сравнению с предыдущим кадром.
 */
void DisplayUI::Flush() {
  for (int i = 0; i < 2; i++) {
    const char* line = screen_.GetLine(i);
    // Оптимизированное сравнение фиксированных 16 символов
    if (memcmp(line, last_lines_[i], 16) != 0) {
      lcd_.setCursor(0, i);
      lcd_.print(line);
      // Копируем все 16 символов + терминатор
      memcpy(last_lines_[i], line, 17);
    }
  }
}

/**
 * @brief Обработка событий кнопок, полученных от ButtonEngine.
 */
void DisplayUI::HandleButtons() {
  // Сброс таймера бездействия и пробуждение подсветки
  if (buttons_.AnyPressed()) {
    needs_redraw_ = true;
    controller_->NotifyUserActivity();
    last_activity_time_ = millis();
    if (!backlight_on_) {
      lcd_.backlight();
      backlight_on_ = true;
      return; // Первое нажатие только пробуждает экран
    }
  }

  ButtonEvent event = buttons_.Poll();
  if (event != ButtonEvent::kNone) {
    needs_redraw_ = true;
    dispatcher_.Dispatch(this, event);
  }
}

/**
 * @brief Автоматическое гашение подсветки при отсутствии активности.
 */
void DisplayUI::UpdateBacklight() {
  if (backlight_on_ && (millis() - last_activity_time_ > kBacklightTimeout)) {
    // В ручном режиме озонирования подсветка не гаснет для безопасности
    if (model_.state != SystemState::kManualOzone) {
      lcd_.noBacklight();
      backlight_on_ = false;
    }
  }
}

/**
 * @brief Диспетчер отрисовки страниц.
 */
void DisplayUI::DrawPage() {
  if (nav_.GetRootIndex() == 0) { // Главный экран
    DrawHomeScreen();
  } else if (!nav_.InSubmenu()) { // Экран выбора раздела
    DrawRootPage();
  } else { // Экран конкретного элемента подменю
    const MenuItemDef* item = GetCurrentItemDef();
    if (item) {
      RenderItem(item);
    }
  }
}

void DisplayUI::RenderItem(const MenuItemDef* item) {
  switch (item->type) {
    case MenuItemType::kView:
      if (item->id == MenuItemID::kErrorView) DrawErrorLog();
      else DrawStatus(item->ctx_index);
      break;
    case MenuItemType::kValue:
      DrawValuePage();
      break;
    case MenuItemType::kAction:
      DrawManualModes();
      break;
    case MenuItemType::kStats:
      DrawStats();
      break;
    default:
      break;
  }
}

/**
 * @brief Отрисовка страницы выбора раздела меню (корень).
 */
void DisplayUI::DrawRootPage() {
  const MenuRootDef* root = GetCurrentRootDef();
  // HOME не считается за пронумерованный раздел в статус-баре
  uint8_t root_idx = nav_.GetRootIndex();
  uint8_t index = (root_idx > 0) ? root_idx - 1 : 0;
  DrawHeader((const __FlashStringHelper*)root->label, index, MENU_TABLE_SIZE - 1);

  screen_.SetPos(1, 0);
  screen_.print(F("  MENU ENTER"));
}

/**
 * @brief Отрисовка стандартного заголовка раздела.
 * Формат: [Метка] [текущий]/[всего]
 */
void DisplayUI::DrawHeader(const char* label, uint8_t index, uint8_t count) {
  screen_.SetPos(0, 0);
  // Приведение к FlashStringHelper, т.к. строки меток лежат в PROGMEM
  screen_.print((const __FlashStringHelper*)label);
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

/**
 * @brief Главный экран: текущие показатели и статус работы.
 */
void DisplayUI::DrawHomeScreen() {
  // Строка 1: Внутри [Т] [H] [Индикатор работы]
  screen_.SetPos(0, 0);
  screen_.print(F("IN "));
  if (model_.inside.valid) screen_.print(model_.inside.temp, 1);
  else screen_.print(F("---"));
  screen_.print(F("C "));
  if (model_.inside.valid) screen_.print(model_.inside.rh, 0);
  else screen_.print(F("---"));
  screen_.print(F("%"));

  screen_.SetPos(0, 15);
  if (model_.ozone_on) screen_.print(F("O"));
  else if (model_.fan_on) screen_.print(F("F"));
  else screen_.print(F(" "));

  // Строка 2: Снаружи [Т] [H] [Режим системы]
  screen_.SetPos(1, 0);
  screen_.print(F("OUT "));
  if (model_.outside.valid) screen_.print(model_.outside.temp, 1);
  else screen_.print(F("---"));
  screen_.print(F("C "));
  if (model_.outside.valid) screen_.print(model_.outside.rh, 0);
  else screen_.print(F("---"));
  screen_.print(F("%"));

  screen_.SetPos(1, 15);
  if (model_.state == SystemState::kErrorState) screen_.print(F("E")); // Ошибка
  else if (model_.is_auto_mode) screen_.print(F("A")); // Авто
  else screen_.print(F("M")); // Ручной
}

void DisplayUI::DrawStatus(int index) {
  if (index == 0) DrawStatusPage(model_.inside, F("IN "));
  else DrawStatusPage(model_.outside, F("OUT "));
}

/**
 * @brief Общий метод отрисовки страницы статуса датчика.
 */
void DisplayUI::DrawStatusPage(const SensorData& data, const __FlashStringHelper* label) {
  const MenuRootDef* root = GetCurrentRootDef();
  DrawHeader(F("STATUS"), nav_.GetItemIndex(), root->item_count);

  screen_.SetPos(1, 0);
  screen_.print(label);
  screen_.print(data.temp, 1);
  screen_.print(F("C "));
  screen_.print(data.rh, 0);
  screen_.print(F("%"));
}


/**
 * @brief Отрисовка страницы выбора устройства для ручного пуска.
 */
void DisplayUI::DrawManualModes() {
  const MenuRootDef* root = GetCurrentRootDef();
  uint8_t item_idx = nav_.GetItemIndex();
  DrawHeader(F("MANUAL"), item_idx, root->item_count);

  screen_.SetPos(1, 0);
  if (item_idx == 0) screen_.print(F(">"));
  else screen_.print(F(" "));
  screen_.print(F("FAN   "));

  if (item_idx == 1) screen_.print(F(">"));
  else screen_.print(F(" "));
  screen_.print(F("OZONE"));
}


void DisplayUI::DrawValuePage() {
  const MenuItemDef* item = GetCurrentItemDef();
  if (!item) return;

  ValuePageDef vcfg;
  memcpy_P(&vcfg, &VALUE_PAGES[item->ctx_index], sizeof(ValuePageDef));

  const MenuRootDef* root = GetCurrentRootDef();
  DrawHeader((const __FlashStringHelper*)root->label, nav_.GetItemIndex(), root->item_count);

  float val = model_.GetValue(vcfg.val_id);

  screen_.SetPos(1, 0);
  screen_.print(F(">"));
  screen_.print((const __FlashStringHelper*)item->label);
  screen_.print(F(":"));

  // Для калибровки добавляем + если значение положительное
  if (item->id >= MenuItemID::kCalibBmeTemp && val > 0.001f) {
    screen_.print(F("+"));
  }

  screen_.print(val, vcfg.precision);
  screen_.print((const __FlashStringHelper*)vcfg.unit);
}

/**
 * @brief Отрисовка статистики наработки или страницы подтверждения сброса.
 */
void DisplayUI::DrawStats() {
  const MenuRootDef* root = GetCurrentRootDef();
  uint8_t item_idx = nav_.GetItemIndex();
  DrawHeader(F("STATS"), item_idx, root->item_count);

  screen_.SetPos(1, 0);
  if (item_idx == 0) { // Просмотр
    screen_.print(F("U"));
    screen_.print(model_.stats.uptimeMinutes / 60);
    screen_.print(F(" F"));
    screen_.print(model_.stats.fanMinutes / 60);
    screen_.print(F(" O3"));
    screen_.print(model_.stats.ozoneMinutes / 60);
  } else if (item_idx == 1) { // Сброс
    screen_.print(F("MENU CONFIRM"));
  }
}

/**
 * @brief Отрисовка лога текущих ошибок.
 */
void DisplayUI::DrawErrorLog() {
  const MenuRootDef* root = GetCurrentRootDef();
  DrawHeader(F("ERRORS"), nav_.GetItemIndex(), root->item_count);

  screen_.SetPos(1, 0);
  if (model_.error == ErrorCode::kNone) {
    screen_.print(F("SYSTEM OK"));
  } else {
    screen_.print(ErrorToString(model_.error));
    screen_.SetPos(1, 12);
    screen_.print(F("UP:R"));
  }
}
