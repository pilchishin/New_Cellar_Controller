#include "DisplayUI.h"
#include "MenuActions.h"
#include "config.h"
#include "Types.h"


// --- ОПРЕДЕЛЕНИЯ ТАБЛИЦ МЕНЮ (Хранятся во Flash-памяти / PROGMEM) ---

// Текстовые метки для элементов статуса
static const char lbl_status_in[] PROGMEM = "STATUS_IN";
static const char lbl_status_out[] PROGMEM = "STATUS_OUT";

/**
 * @brief Элементы раздела "STATUS".
 * Позволяют просматривать детальные данные внутреннего и внешнего датчиков.
 */
static const MenuItemDef STATUS_ITEMS[] PROGMEM = {
  { MenuItemID::kStatusIn,  lbl_status_in,  MenuItemType::kView, 0, ActionID::kNavPrev, ActionID::kNavNext, ActionID::kNavNext, ActionID::kExitSubmenu },
  { MenuItemID::kStatusOut, lbl_status_out, MenuItemType::kView, 1, ActionID::kNavPrev, ActionID::kNavNext, ActionID::kNavNext, ActionID::kExitSubmenu }
};

// --- КОНФИГУРАЦИЯ СТРАНИЦ РЕДАКТИРОВАНИЯ ---

// Единицы измерения
static const char unit_c[] PROGMEM = "C";
static const char unit_pct[] PROGMEM = "%";

/**
 * @brief Параметры настройки числовых величин.
 * Формат: { ID элемента, ID переменной в модели, Мин, Макс, Шаг, Ед.изм., Точность }
 */
const ValuePageDef VALUE_PAGES[] PROGMEM = {
  { MenuItemID::kTargetTemp,   ValueID::kTargetTemp,   kTempCriticalMin, 15.0f, 0.1f, unit_c,   1 },
  { MenuItemID::kTargetHum,    ValueID::kTargetHum,    50.0f, 95.0f, 1.0f, unit_pct, 0 },
  { MenuItemID::kCalibBmeTemp, ValueID::kCalibBmeTemp, -5.0f,  5.0f, 0.1f, unit_c,   1 },
  { MenuItemID::kCalibBmeHum,  ValueID::kCalibBmeHum,  -5.0f,  5.0f, 0.1f, unit_pct, 1 },
  { MenuItemID::kCalibHtuTemp, ValueID::kCalibHtuTemp, -5.0f,  5.0f, 0.1f, unit_c,   1 },
  { MenuItemID::kCalibHtuHum,  ValueID::kCalibHtuHum,  -5.0f,  5.0f, 0.1f, unit_pct, 1 },
  { MenuItemID::kCalibDsTemp,  ValueID::kCalibDsTemp,  -5.0f,  5.0f, 0.1f, unit_c,   1 }
};

// Элементы раздела "TARGETS" (Уставки)
static const char lbl_target_t[] PROGMEM = "TEMP";
static const char lbl_target_h[] PROGMEM = "HUM";
static const MenuItemDef TARGET_ITEMS[] PROGMEM = {
  { MenuItemID::kTargetTemp, lbl_target_t, MenuItemType::kValue, 0, ActionID::kValueInc, ActionID::kValueDec, ActionID::kNavNext, ActionID::kExitSubmenu },
  { MenuItemID::kTargetHum,  lbl_target_h, MenuItemType::kValue, 1, ActionID::kValueInc, ActionID::kValueDec, ActionID::kNavNext, ActionID::kExitSubmenu }
};

// Элементы раздела "MANUAL" (Ручной режим)
static const char lbl_man_fan[] PROGMEM = "MANUAL_FAN";
static const char lbl_man_o3[] PROGMEM = "MANUAL_OZONE";
static const MenuItemDef MANUAL_ITEMS[] PROGMEM = {
  { MenuItemID::kManualFan,   lbl_man_fan,   MenuItemType::kAction, 0, ActionID::kNavPrev, ActionID::kNavNext, ActionID::kManualStart, ActionID::kExitSubmenu },
  { MenuItemID::kManualOzone, lbl_man_o3,    MenuItemType::kAction, 0, ActionID::kNavPrev, ActionID::kNavNext, ActionID::kManualStart, ActionID::kExitSubmenu }
};

// Элементы раздела "STATS" (Статистика)
static const char lbl_stats_v[] PROGMEM = "STATS_VIEW";
static const char lbl_stats_r[] PROGMEM = "STATS_RESET";
static const MenuItemDef STATS_ITEMS[] PROGMEM = {
  { MenuItemID::kStatsView,  lbl_stats_v, MenuItemType::kStats, 0, ActionID::kNavPrev,    ActionID::kNavNext, ActionID::kNavNext,     ActionID::kExitSubmenu },
  { MenuItemID::kStatsReset, lbl_stats_r, MenuItemType::kStats, 0, ActionID::kNavPrev,    ActionID::kNavNext, ActionID::kNavNext,     ActionID::kStatsReset }
};

// Элементы раздела "ERRORS" (Ошибки)
static const char lbl_err_v[] PROGMEM = "ERROR_VIEW";
static const MenuItemDef ERROR_ITEMS[] PROGMEM = {
  { MenuItemID::kErrorView, lbl_err_v, MenuItemType::kView, 0, ActionID::kErrorReset, ActionID::kNone,    ActionID::kNavNext,     ActionID::kExitSubmenu }
};

// Элементы раздела "SERVICE" (Калибровка)
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
 * Каждый раздел определяет свой набор элементов, действия кнопок на корневом уровне
 * и интервал фонового обновления данных на экране.
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

// Вычисление размера таблицы разделов на этапе компиляции
static const uint8_t MENU_TABLE_SIZE = sizeof(MENU_TABLE) / sizeof(MENU_TABLE[0]);

uint8_t DisplayUI::GetMenuTableSize() const { return MENU_TABLE_SIZE; }


/**
 * @brief Конструктор модуля интерфейса.
 * Инициализирует драйвер LCD (адрес 0x27), менеджеры навигации и ввода.
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
  // Очистка кэша строк для корректной первой отрисовки
  memset(last_lines_, 0, sizeof(last_lines_));
}

/**
 * @brief Начальная инициализация периферии.
 */
void DisplayUI::Init() {
  lcd_.init();
  lcd_.backlight();
  buttons_.Init();
  last_activity_time_ = millis();
}

/**
 * @brief Аварийное восстановление дисплея.
 * Используется контроллером при обнаружении «подвисания» шины I2C.
 */
void DisplayUI::Reinit() {
  lcd_.init();
  if (backlight_on_)
    lcd_.backlight();
  else
    lcd_.noBacklight();
}

// Резервирование памяти под статические буферы для работы с Flash
MenuRootDef DisplayUI::current_root_buf_;
MenuItemDef DisplayUI::current_item_buf_;

/**
 * @brief Безопасное чтение текущего корневого раздела из PROGMEM.
 */
const MenuRootDef* DisplayUI::GetCurrentRootDef() const {
  memcpy_P(&current_root_buf_, &MENU_TABLE[nav_.GetRootIndex()], sizeof(MenuRootDef));
  return &current_root_buf_;
}

/**
 * @brief Безопасное чтение текущего элемента подменю из PROGMEM.
 * Сначала извлекается указатель на массив элементов, затем копируется сама структура.
 */
const MenuItemDef* DisplayUI::GetCurrentItemDef() const {
  uint8_t root_idx = nav_.GetRootIndex();
  uint8_t item_idx = nav_.GetItemIndex();

  // Чтение указателя на подменю из структуры корня, лежащей во Flash
  const MenuItemDef* items_ptr = (const MenuItemDef*)pgm_read_ptr(&MENU_TABLE[root_idx].items);
  uint8_t count = pgm_read_byte(&MENU_TABLE[root_idx].item_count);

  if (items_ptr && item_idx < count) {
    // Копирование структуры элемента из Flash в RAM-буфер
    memcpy_P(&current_item_buf_, &items_ptr[item_idx], sizeof(MenuItemDef));
    return &current_item_buf_;
  }
  return nullptr;
}

/**
 * @brief Основной итератор интерфейса.
 * Синхронизирует данные, обрабатывает ввод и управляет циклом перерисовки.
 */
void DisplayUI::Update() {
  // Обновление локальной модели данных из датчиков и контроллера
  model_.Sync(controller_, sensors_);

  HandleButtons();    // Опрос кнопок
  UpdateBacklight();  // Проверка тайм-аута гашения

  static unsigned long last_draw = 0;
  const MenuRootDef* root = GetCurrentRootDef();
  unsigned long interval = root ? root->refresh_interval_ms : 1000;

  // Отрисовка кадра: либо принудительно (кнопки), либо по таймеру обновления данных
  if (needs_redraw_ || (millis() - last_draw >= interval)) {
    last_draw = millis();
    needs_redraw_ = false;

    screen_.Clear(); // Очистка виртуального холста

    // Приоритет: отображение временного уведомления (например, "SAVED")
    if (temp_message_ != nullptr && millis() - message_timer_ < 2000) {
      screen_.SetPos(0, 0);
      screen_.print(temp_message_);
      screen_.SetPos(1, 0);
      screen_.print(F("                "));
    } else {
      if (temp_message_ != nullptr) {
        temp_message_ = nullptr;
      }
      DrawPage(); // Наполнение холста содержимым текущей страницы меню
    }

    Flush(); // Оптимизированный вывод изменений на физический LCD
  }
}

/**
 * @brief Интеллектуальный вывод на LCD по шине I2C.
 * Сравнивает каждую строку с предыдущим состоянием и обновляет на экране только
 * изменившиеся символы (строки). Это существенно снижает нагрузку на медленную шину I2C.
 */
void DisplayUI::Flush() {
  for (int i = 0; i < 2; i++) {
    const char* line = screen_.GetLine(i);
    // Построчное сравнение буфера и кэша экрана
    if (memcmp(line, last_lines_[i], 16) != 0) {
      lcd_.setCursor(0, i);
      lcd_.print(line);
      // Обновление кэша (включая терминатор для безопасности)
      memcpy(last_lines_[i], line, 17);
    }
  }
}

/**
 * @brief Реакция на физическое нажатие кнопок.
 * Автоматически включает подсветку и делегирует событие диспетчеру меню.
 */
void DisplayUI::HandleButtons() {
  if (buttons_.AnyPressed()) {
    needs_redraw_ = true;
    controller_->NotifyUserActivity(); // Сообщаем системе, что человек рядом
    last_activity_time_ = millis();

    // Если экран был погашен, первое нажатие только включает свет
    if (!backlight_on_) {
      lcd_.backlight();
      backlight_on_ = true;
      return;
    }
  }

  // Получение логического события от ButtonEngine (click, long click, repeat)
  ButtonEvent event = buttons_.Poll();
  if (event != ButtonEvent::kNone) {
    needs_redraw_ = true;
    dispatcher_.Dispatch(this, event); // Передача события в логику навигации/действий
  }
}

/**
 * @brief Управление энергосбережением.
 * Гасит подсветку через kBacklightTimeout мс, если нет активности.
 */
void DisplayUI::UpdateBacklight() {
  if (backlight_on_ && (millis() - last_activity_time_ > kBacklightTimeout)) {
    // ВАЖНО: При ручном озонировании подсветка НЕ гаснет (требование безопасности ТЗ)
    if (model_.GetState() != SystemState::kManualOzone) {
      lcd_.noBacklight();
      backlight_on_ = false;
    }
  }
}

/**
 * @brief Главный диспетчер шаблонов страниц.
 * Выбирает нужный метод отрисовки в зависимости от глубины навигации.
 */
void DisplayUI::DrawPage() {
  if (nav_.GetRootIndex() == 0) {
    DrawHomeScreen(); // Главная (индекс 0)
  } else if (!nav_.InSubmenu()) {
    DrawRootPage();   // Выбор раздела (корень 1-N)
  } else {
    // Мы внутри раздела — рисуем текущий элемент
    const MenuItemDef* item = GetCurrentItemDef();
    if (item) {
      RenderItem(item);
    }
  }
}

/**
 * @brief Централизованный вызов рендереров по типам элементов.
 */
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
 * @brief Отрисовка страницы выбора корневого раздела.
 */
void DisplayUI::DrawRootPage() {
  const MenuRootDef* root = GetCurrentRootDef();
  // Экран HOME не считается порядковым номером в полосе прокрутки
  uint8_t root_idx = nav_.GetRootIndex();
  uint8_t index = (root_idx > 0) ? root_idx - 1 : 0;
  DrawHeader((const __FlashStringHelper*)root->label, index, MENU_TABLE_SIZE - 1);

  screen_.SetPos(1, 0);
  screen_.print(F("  MENU ENTER"));
}

/**
 * @brief Формирование стандартного заголовка с индикатором позиции.
 * @param label Текст заголовка.
 * @param index Текущий индекс (0-based).
 * @param count Общее количество элементов.
 */
void DisplayUI::DrawHeader(const char* label, uint8_t index, uint8_t count) {
  screen_.SetPos(0, 0);
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
 * @brief Шаблон ГЛАВНОГО ЭКРАНА.
 * Отображает сводку по подвалу и улице, а также индикаторы активных реле и режима.
 */
void DisplayUI::DrawHomeScreen() {
  // Строка 1: Показатели внутри подвала [T] [H] [Индикатор работы]
  screen_.SetPos(0, 0);
  screen_.print(F("IN "));
  if (model_.GetInside().valid) screen_.print(model_.GetInside().temp, 1);
  else screen_.print(F("---"));
  screen_.print(F("C "));
  if (model_.GetInside().valid) screen_.print(model_.GetInside().rh, 0);
  else screen_.print(F("---"));
  screen_.print(F("%"));

  // Индикаторы активного оборудования (O — Озон, F — Вентилятор)
  screen_.SetPos(0, 15);
  if (model_.IsOzoneOn()) screen_.print(F("O"));
  else if (model_.IsFanOn()) screen_.print(F("F"));
  else screen_.print(F(" "));

  // Строка 2: Показатели на улице [T] [H] [Режим системы]
  screen_.SetPos(1, 0);
  screen_.print(F("OUT "));
  if (model_.GetOutside().valid) screen_.print(model_.GetOutside().temp, 1);
  else screen_.print(F("---"));
  screen_.print(F("C "));
  if (model_.GetOutside().valid) screen_.print(model_.GetOutside().rh, 0);
  else screen_.print(F("---"));
  screen_.print(F("%"));

  // Режим системы (E — Ошибка, A — Авто, M — Ручной)
  screen_.SetPos(1, 15);
  if (model_.GetState() == SystemState::kErrorState) screen_.print(F("E"));
  else if (model_.IsAutoMode()) screen_.print(F("A"));
  else screen_.print(F("M"));
}

/**
 * @brief Отрисовка страницы детального статуса датчика.
 */
void DisplayUI::DrawStatus(int index) {
  if (index == 0) DrawStatusPage(model_.GetInside(), F("IN "));
  else DrawStatusPage(model_.GetOutside(), F("OUT "));
}

/**
 * @brief Внутренний шаблон вывода параметров микроклимата.
 */
void DisplayUI::DrawStatusPage(const SensorData& data, const __FlashStringHelper* label) {
  const MenuRootDef* root = GetCurrentRootDef();
  DrawHeader(F("STATUS"), nav_.GetItemIndex(), root->item_count);

  screen_.SetPos(1, 0);
  screen_.print(label);
  if (data.valid) screen_.print(data.temp, 1);
  else screen_.print(F("---"));
  screen_.print(F("C "));
  if (data.valid) screen_.print(data.rh, 0);
  else screen_.print(F("---"));
  screen_.print(F("%"));
}

/**
 * @brief Шаблон страницы запуска ручных режимов.
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

/**
 * @brief Универсальный шаблон страницы редактирования числового значения.
 * Читает конфигурацию (шаг, границы) из Flash и текущее значение из модели.
 */
void DisplayUI::DrawValuePage() {
  const MenuItemDef* item = GetCurrentItemDef();
  if (!item) return;

  // Загрузка параметров страницы редактирования из Flash
  ValuePageDef vcfg;
  memcpy_P(&vcfg, &VALUE_PAGES[item->ctx_index], sizeof(ValuePageDef));

  const MenuRootDef* root = GetCurrentRootDef();
  DrawHeader((const __FlashStringHelper*)root->label, nav_.GetItemIndex(), root->item_count);

  float val = model_.GetValue(vcfg.val_id);

  screen_.SetPos(1, 0);
  screen_.print(F(">"));
  screen_.print((const __FlashStringHelper*)item->label);
  screen_.print(F(":"));

  // Спец. форматирование для калибровок (вывод знака +)
  if (item->id >= MenuItemID::kCalibBmeTemp && val > 0.001f) {
    screen_.print(F("+"));
  }

  screen_.print(val, vcfg.precision);
  screen_.print((const __FlashStringHelper*)vcfg.unit);
}

/**
 * @brief Шаблон страницы статистики наработки.
 * Реализует сокращенный вывод (U - uptime, F - Fan, O3 - Ozone).
 */
void DisplayUI::DrawStats() {
  const MenuRootDef* root = GetCurrentRootDef();
  uint8_t item_idx = nav_.GetItemIndex();
  DrawHeader(F("STATS"), item_idx, root->item_count);

  screen_.SetPos(1, 0);
  if (item_idx == 0) { // Просмотр времени наработки (в часах)
    screen_.print(F("U"));
    screen_.print(model_.GetStats().uptimeMinutes / 60);
    screen_.print(F(" F"));
    screen_.print(model_.GetStats().fanMinutes / 60);
    screen_.print(F(" O3"));
    screen_.print(model_.GetStats().ozoneMinutes / 60);
  } else if (item_idx == 1) { // Страница подтверждения сброса
    screen_.print(F("MENU CONFIRM"));
  }
}

/**
 * @brief Шаблон страницы просмотра системных ошибок.
 */
void DisplayUI::DrawErrorLog() {
  const MenuRootDef* root = GetCurrentRootDef();
  DrawHeader(F("ERRORS"), nav_.GetItemIndex(), root->item_count);

  screen_.SetPos(1, 0);
  if (model_.GetError() == ErrorCode::kNone) {
    screen_.print(F("SYSTEM OK"));
  } else {
    // Вывод текстового описания ошибки
#ifdef DEBUG
    screen_.print(ErrorToString(model_.GetError()));
#endif
    // Подсказка для сброса (UP — сброс)
    screen_.SetPos(1, 12);
    screen_.print(F("UP:R"));
  }
}
