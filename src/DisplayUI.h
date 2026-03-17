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
 * @brief Класс для формирования содержимого экрана 16x2.
 * Реализует интерфейс Print для удобного вывода текста в строковый буфер.
 */
class ScreenBuffer : public Print {
 public:
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
   * @brief Установка позиции курсора в буфере.
   * @param row Строка (0-1).
   * @param col Колонка (0-15).
   */
  void SetPos(int row, int col) {
    if (row >= 0 && row < 2) row_ = row;
    if (col >= 0 && col < 16) col_ = col;
  }

  /**
   * @brief Запись одного символа в текущую позицию буфера.
   */
  size_t write(uint8_t c) override {
    if (col_ < 16) {
      buffer[row_][col_++] = (char)c;
      return 1;
    }
    return 0;
  }

  /**
   * @brief Получение указателя на строку буфера.
   * @param row Номер строки.
   */
  const char* GetLine(int row) const {
    if (row < 0 || row >= 2) return "";
    return buffer[row];
  }

 private:
  char buffer[2][17]; // 16 символов + терминатор для двух строк
  int row_, col_;     // Текущая позиция курсора в буфере
};

/**
 * @brief Идентификаторы элементов меню для быстрой идентификации в коде.
 */
enum class MenuItemID : uint8_t {
  kNone,
  kStatusIn,
  kStatusOut,
  kTargetTemp,
  kTargetHum,
  kManualFan,
  kManualOzone,
  kStatsView,
  kStatsReset,
  kErrorView,
  kCalibBmeTemp,
  kCalibBmeHum,
  kCalibHtuTemp,
  kCalibHtuHum,
  kCalibDsTemp
};

/**
 * @brief Определение параметров страницы редактирования числового значения.
 */
struct ValuePageDef {
  MenuItemID id;            // ID элемента меню
  float UIModel::*val_ptr;  // Указатель на поле в UIModel
  float min_val;            // Минимальное значение
  float max_val;            // Максимальное значение
  float step;               // Шаг изменения
  const char* unit;         // Единица измерения (в PROGMEM)
  uint8_t precision;        // Количество знаков после запятой
};

/**
 * @brief Определение элемента подменю.
 */
struct MenuItemDef {
  MenuItemID id;                  // Уникальный идентификатор элемента
  const char* label;              // Метка элемента (в PROGMEM)
  void (*draw)(DisplayUI* ui);    // Функция отрисовки содержимого
  void (*on_up)(DisplayUI* ui);   // Обработчик кнопки ВВЕРХ
  void (*on_down)(DisplayUI* ui); // Обработчик кнопки ВНИЗ
  void (*on_menu)(DisplayUI* ui); // Обработчик короткого нажатия МЕНЮ
  void (*on_long_menu)(DisplayUI* ui); // Обработчик долгого нажатия МЕНЮ
};

/**
 * @brief Определение корневого раздела меню.
 */
struct MenuRootDef {
  const char* label;              // Название раздела (в PROGMEM)
  const MenuItemDef* items;       // Указатель на массив элементов подменю (в PROGMEM)
  uint8_t item_count;             // Количество элементов в подразделе
  void (*on_up)(DisplayUI* ui);   // Обработчик кнопки ВВЕРХ на уровне корня
  void (*on_down)(DisplayUI* ui); // Обработчик кнопки ВНИЗ на уровне корня
  void (*on_menu)(DisplayUI* ui); // Обработчик короткого нажатия МЕНЮ
  void (*on_long_menu)(DisplayUI* ui); // Обработчик долгого нажатия МЕНЮ
  uint16_t refresh_interval_ms;   // Интервал обновления экрана для этого раздела
};

/**
 * @brief Класс управления пользовательским интерфейсом на базе LCD 16x2.
 * Реализует древовидное меню, обработку кнопок и вывод данных датчиков.
 */
class DisplayUI {
 private:
  LiquidCrystal_I2C lcd_;       // Объект управления LCD по I2C
  Controller* controller_;      // Ссылка на основной контроллер системы
  SensorManager* sensors_;      // Ссылка на менеджер датчиков
  TimeManager* rtc_;            // Ссылка на менеджер времени
  MenuNavigator nav_;           // Состояние навигации по меню
  ButtonEngine buttons_;        // Обработчик кнопок
  MenuDispatcher dispatcher_;   // Маршрутизатор событий меню
  UIModel model_;               // Локальная копия данных для отображения

  // Временные уведомления на экране
  unsigned long message_timer_;    // Таймер отображения сообщения
  const char* temp_message_;       // Текст временного сообщения

  // Управление подсветкой
  unsigned long last_activity_time_; // Время последнего действия пользователя
  bool backlight_on_;                // Флаг состояния подсветки

  // Оптимизация вывода (выводятся только изменившиеся строки)
  ScreenBuffer screen_;              // Буфер текущего кадра
  char last_lines_[2][17];           // Копия предыдущего кадра для сравнения
  bool needs_redraw_;                // Флаг принудительной перерисовки
  void Flush();                      // Перенос данных из буфера на физический экран

  void HandleButtons();    // Опрос физических кнопок
  void DrawPage();         // Определение того, какую страницу рисовать
  void DrawRootPage();     // Отрисовка страницы выбора раздела
  void UpdateBacklight();  // Управление тайм-аутом подсветки

  // Вспомогательные методы отрисовки заголовков
  void DrawHeader(const char* label, uint8_t index, uint8_t count);
  void DrawHeader(const __FlashStringHelper* label, uint8_t index, uint8_t count);

  // Методы отрисовки конкретных страниц
  void DrawHomeScreen();    // Главный экран со статусом
  void DrawStatusIn();      // Внутренние показатели
  void DrawStatusOut();     // Внешние показатели
  void DrawStatusPage(const SensorData& data, const __FlashStringHelper* label); // Общий метод отрисовки статуса
  void DrawManualModes();   // Ручное управление устройствами
  void DrawValuePage();     // Универсальная страница редактирования значения
  void DrawStats();         // Просмотр статистики наработки
  void DrawErrorLog();      // Просмотр лога ошибок

  uint8_t GetMenuTableSize() const;

  const ValuePageDef* GetValuePageDef(MenuItemID id) const;

  friend class MenuActions;
  friend class MenuDispatcher;

 public:
  // Получение определений текущего положения в меню
  const MenuRootDef* GetCurrentRootDef() const;
  const MenuItemDef* GetCurrentItemDef() const;

  // Буферы для чтения структур из Flash-памяти
  static MenuRootDef current_root_buf_;
  static MenuItemDef current_item_buf_;

 public:
  DisplayUI(Controller* c, SensorManager* s, TimeManager* t);
  void Init();
  void Reinit();  // Повторная инициализация LCD (например, после сбоя I2C)
  void Update();  // Основной цикл обновления UI (вызывается в loop)

  /**
   * @brief Проверка состояния подсветки.
   */
  bool IsBacklightOn() const { return backlight_on_; }
};

#endif
