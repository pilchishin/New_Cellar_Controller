#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Controller.h"
#include "SensorManager.h"
#include "TimeManager.h"
#include "MenuNavigator.h"
#include "ButtonEngine.h"
#include "MenuDispatcher.h"
#include "UIModel.h"

class DisplayUI;

/**
 * @class ScreenBuffer
 * @brief Вспомогательный класс для формирования содержимого экрана 16x2.
 * Реализует интерфейс Print для удобного вывода текста, чисел и форматированных строк
 * во внутренний двухстрочный буфер перед отправкой на физический дисплей.
 */
class ScreenBuffer : public Print {
 public:
  /**
   * @brief Конструктор. Инициализирует пустой буфер.
   */
  ScreenBuffer() { Clear(); }

  /**
   * @brief Очистка буфера (заполнение пробелами).
   */
  void Clear() {
    memset(buffer[0], ' ', 16);
    buffer[0][16] = '\0';
    memset(buffer[1], ' ', 16);
    buffer[1][16] = '\0';
    row_ = 0;
    col_ = 0;
  }

  /**
   * @brief Установка виртуальной позиции курсора в буфере.
   * @param row Строка (0 или 1).
   * @param col Колонка (0-15).
   */
  void SetPos(int row, int col) {
    if (row >= 0 && row < 2) row_ = row;
    if (col >= 0 && col < 16) col_ = col;
  }

  /**
   * @brief Запись одного символа в текущую позицию буфера.
   * Требуется для реализации интерфейса Print.
   */
  size_t write(uint8_t c) override {
    if (col_ < 16) {
      buffer[row_][col_++] = (char)c;
      return 1;
    }
    return 0;
  }

  /**
   * @brief Получение указателя на сформированную строку буфера.
   * @param row Номер строки (0 или 1).
   * @return Указатель на C-строку длиной 16 символов.
   */
  const char* GetLine(int row) const {
    if (row < 0 || row >= 2) return "";
    return buffer[row];
  }

 private:
  char buffer[2][17]; ///< 16 символов + нулевой терминатор для двух строк
  int row_, col_;     ///< Текущие координаты "печати" в буфере
};

/**
 * @enum ActionID
 * @brief Идентификаторы высокоуровневых действий, привязанных к кнопкам меню.
 */
enum class ActionID : uint8_t {
  kNone,           ///< Действие не назначено
  kNavNext,        ///< Переход к следующему элементу подменю
  kNavPrev,        ///< Переход к предыдущему элементу подменю
  kNavNextRoot,    ///< Переход к следующему корневому разделу
  kNavPrevRoot,    ///< Переход к предыдущему корневому разделу
  kValueInc,       ///< Увеличение редактируемого значения
  kValueDec,       ///< Уменьшение редактируемого значения
  kEnterSubmenu,   ///< Вход в выбранный подраздел
  kExitSubmenu,    ///< Возврат на уровень вверх
  kManualStart,    ///< Запуск устройства в ручном режиме
  kStatsReset,     ///< Сброс накопленной статистики
  kErrorReset      ///< Попытка сброса активной ошибки
};

/**
 * @enum MenuItemType
 * @brief Типы элементов меню, определяющие логику их отрисовки и поведения.
 */
enum class MenuItemType : uint8_t {
  kView,    ///< Только чтение (статус датчиков, список ошибок)
  kValue,   ///< Редактируемое число (уставки, калибровки)
  kAction,  ///< Кнопка действия (запуск ручных режимов)
  kStats    ///< Специальная страница отображения наработки (моточасы)
};

/**
 * @enum MenuItemID
 * @brief Уникальные идентификаторы страниц и элементов меню для логики управления.
 */
enum class MenuItemID : uint8_t {
  kNone,
  kStatusIn,     ///< Статус внутреннего датчика
  kStatusOut,    ///< Статус внешнего датчика
  kTargetTemp,   ///< Настройка целевой температуры
  kTargetHum,    ///< Настройка целевой влажности
  kManualFan,    ///< Ручной пуск вентилятора
  kManualOzone,  ///< Ручной пуск озонатора
  kStatsView,    ///< Просмотр статистики
  kStatsReset,   ///< Сброс статистики
  kErrorView,    ///< Просмотр текущей ошибки
  kCalibBmeTemp, ///< Калибровочные смещения...
  kCalibBmeHum,
  kCalibHtuTemp,
  kCalibHtuHum,
  kCalibDsTemp
};

/**
 * @struct ValuePageDef
 * @brief Определение параметров страницы редактирования числового значения.
 * Хранится в PROGMEM для экономии оперативной памяти.
 */
struct ValuePageDef {
  MenuItemID id;            ///< ID элемента меню
  ValueID val_id;           ///< Соответствующее значение в UIModel
  float min_val;            ///< Нижняя граница регулировки
  float max_val;            ///< Верхняя граница регулировки
  float step;               ///< Шаг изменения при одном нажатии
  const char* unit;         ///< Строка единицы измерения (в PROGMEM)
  uint8_t precision;        ///< Количество знаков после запятой
};

/**
 * @struct MenuItemDef
 * @brief Определение элемента подменю (строка внутри раздела).
 * Описывает визуальную метку и привязанные действия для кнопок.
 */
struct MenuItemDef {
  MenuItemID id;                  ///< Уникальный идентификатор
  const char* label;              ///< Текстовая метка (в PROGMEM)
  MenuItemType type;              ///< Способ отображения
  uint8_t ctx_index;              ///< Индекс в массиве данных (для калибровок/датчиков)
  ActionID up_action;             ///< Что делать при нажатии ВВЕРХ
  ActionID down_action;           ///< Что делать при нажатии ВНИЗ
  ActionID menu_action;           ///< Что делать при коротком нажатии МЕНЮ
  ActionID long_menu_action;      ///< Что делать при долгом нажатии МЕНЮ
};

/**
 * @struct MenuRootDef
 * @brief Определение корневого раздела меню (верхний уровень навигации).
 */
struct MenuRootDef {
  const char* label;              ///< Название раздела (в PROGMEM)
  const MenuItemDef* items;       ///< Указатель на массив дочерних элементов (в PROGMEM)
  uint8_t item_count;             ///< Количество элементов в этом разделе
  ActionID up_action;             ///< Действие кнопки ВВЕРХ на уровне корня
  ActionID down_action;           ///< Действие кнопки ВНИЗ на уровне корня
  ActionID menu_action;           ///< Действие короткого нажатия МЕНЮ
  ActionID long_menu_action;      ///< Действие долгого нажатия МЕНЮ
  uint16_t refresh_interval_ms;   ///< Частота обновления экрана в этом режиме
};

/**
 * @class DisplayUI
 * @brief Модуль управления пользовательским интерфейсом (LCD + Кнопки).
 * Реализует архитектуру на базе табличного меню, хранящегося во Flash-памяти.
 * Обеспечивает неблокирующую отрисовку, оптимизацию вывода по I2C и
 * обработку высокоуровневых событий навигации.
 */
class DisplayUI {
 private:
  LiquidCrystal_I2C lcd_;       ///< Низкоуровневый драйвер LCD
  Controller* controller_;      ///< Ссылка на ядро системы
  SensorManager* sensors_;      ///< Доступ к данным датчиков
  TimeManager* rtc_;            ///< Доступ к реальному времени
  MenuNavigator nav_;           ///< Хранитель состояния навигации (Root/Item)
  ButtonEngine buttons_;        ///< Обработчик физических кнопок
  MenuDispatcher dispatcher_;   ///< Перевод событий кнопок в действия меню
  UIModel model_;               ///< Локальное хранилище данных для отображения

  // --- Временные уведомления ---
  unsigned long message_timer_;    ///< Время начала показа уведомления
  const char* temp_message_;       ///< Текст текущего сообщения (если есть)

  // --- Энергосбережение ---
  unsigned long last_activity_time_; ///< Время последнего нажатия кнопки
  bool backlight_on_;                ///< Текущее состояние подсветки

  // --- Оптимизация отрисовки ---
  ScreenBuffer screen_;              ///< Виртуальный холст для отрисовки кадра
  char last_lines_[2][17];           ///< Кэш предыдущего кадра для поиска изменений
  bool needs_redraw_;                ///< Флаг принудительного обновления кадра

  // --- Состояние навигации ошибок ---
  uint8_t error_page_index_;         ///< Текущая страница в разделе ERRORS
  uint8_t error_page_count_;         ///< Общее кол-во страниц ошибок + RESET
  bool error_reset_pending_;         ///< Флаг процесса долгого нажатия MENU
  unsigned long error_reset_press_start_; ///< Метка времени начала нажатия
  unsigned long last_blink_ms_;      ///< Таймер мигания '!' на главном экране

  /**
   * @brief Возвращает код ошибки для конкретной страницы.
   */
  ErrorCode ErrorPageAt(uint8_t p, ErrorMask mask) const;

  /**
   * @brief Перенос данных из виртуального буфера на физический дисплей.
   * Отправляет по I2C только те строки, содержимое которых реально изменилось.
   */
  void Flush();

  void HandleButtons();    ///< Опрос кнопок и пробуждение подсветки
  void DrawPage();         ///< Определение типа текущей страницы для отрисовки
  void DrawRootPage();     ///< Отрисовка страницы выбора раздела меню
  void UpdateBacklight();  ///< Проверка таймаута автоматического гашения экрана

  // --- Вспомогательные методы формирования заголовков ---
  void DrawHeader(const char* label, uint8_t index, uint8_t count);
  void DrawHeader(const __FlashStringHelper* label, uint8_t index, uint8_t count);

  // --- Методы отрисовки конкретных шаблонов страниц ---
  void DrawHomeScreen();    ///< Главный экран (показатели IN/OUT и режим)
  void RenderItem(const MenuItemDef* item); ///< Рендерер содержимого подменю
  void DrawStatus(int index); ///< Переключатель страниц статуса (IN=0, OUT=1)
  void DrawStatusPage(const SensorData& data, const __FlashStringHelper* label); ///< Шаблон данных датчика
  void DrawManualModes();   ///< Экран управления FAN/OZONE
  void DrawValuePage();     ///< Универсальный экран редактирования числовых параметров
  void DrawStats();         ///< Экран отображения моточасов системы
  void DrawErrorScreen();   ///< Экран просмотра журнала ошибок

  /**
   * @brief Возвращает количество корневых разделов меню.
   */
  uint8_t GetMenuTableSize() const;

  friend class MenuActions;
  friend class MenuDispatcher;

 public:
  /**
   * @brief Чтение текущего корня меню из PROGMEM в буфер RAM.
   */
  const MenuRootDef* GetCurrentRootDef() const;

  /**
   * @brief Чтение текущего элемента подменю из PROGMEM в буфер RAM.
   */
  const MenuItemDef* GetCurrentItemDef() const;

  // Статические буферы для исключения динамического выделения памяти при работе с Flash
  static MenuRootDef current_root_buf_;
  static MenuItemDef current_item_buf_;

 public:
  /**
   * @brief Конструктор модуля интерфейса.
   */
  DisplayUI(Controller* c, SensorManager* s, TimeManager* t);

  /**
   * @brief Начальная инициализация железа.
   */
  void Init();

  /**
   * @brief Повторная инициализация LCD после критического сбоя шины I2C.
   */
  void Reinit();

  /**
   * @brief Главный итерационный метод UI. Должен вызываться в loop().
   */
  void Update();

  /**
   * @brief Проверка текущего состояния подсветки.
   */
  bool IsBacklightOn() const { return backlight_on_; }
};

/**
 * @brief Таблица конфигураций страниц редактирования значений (в PROGMEM).
 */
extern const ValuePageDef VALUE_PAGES[] PROGMEM;

#endif
