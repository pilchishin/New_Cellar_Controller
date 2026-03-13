#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Controller.h"
#include "SensorManager.h"
#include "TimeManager.h"

class DisplayUI;

/**
 * @brief Класс для формирования содержимого экрана 16x2.
 */
class ScreenBuffer : public Print {
 public:
  ScreenBuffer() { Clear(); }

  void Clear() {
    memset(lines_[0], ' ', 16);
    lines_[0][16] = '\0';
    memset(lines_[1], ' ', 16);
    lines_[1][16] = '\0';
    row_ = 0;
    col_ = 0;
  }

  void SetPos(int row, int col) {
    if (row >= 0 && row < 2) row_ = row;
    if (col >= 0 && col < 16) col_ = col;
  }

  size_t write(uint8_t c) override {
    if (col_ < 16) {
      lines_[row_][col_++] = (char)c;
      return 1;
    }
    return 0;
  }

  const char* GetLine(int row) const {
    if (row < 0 || row >= 2) return "";
    return lines_[row];
  }

 private:
  char lines_[2][17];
  int row_, col_;
};

// Разделы меню верхнего уровня
enum class MenuRoot {
  HOME,
  STATUS,
  TARGETS,
  MANUAL,
  STATS,
  ERRORS,
  SERVICE
};

// Элементы подменю
enum class MenuItem {
  NONE,

  STATUS_IN,
  STATUS_OUT,

  TARGET_TEMP,
  TARGET_HUM,

  MANUAL_FAN,
  MANUAL_OZONE,

  STATS_VIEW,
  STATS_RESET,

  ERROR_VIEW,

  CALIB_BME_T,
  CALIB_BME_H,
  CALIB_HTU_T,
  CALIB_HTU_H,
  CALIB_DS_T
};

struct MenuItemDef {
  MenuItem id;
  const char* label;
  void (*draw)(DisplayUI* ui);
  void (*on_up)(DisplayUI* ui);
  void (*on_down)(DisplayUI* ui);
  void (*on_menu)(DisplayUI* ui);
  void (*on_long_menu)(DisplayUI* ui);
};

struct MenuRootDef {
  MenuRoot id;
  const char* label;
  const MenuItemDef* items;
  uint8_t item_count;
};

class DisplayUI {
 private:
  LiquidCrystal_I2C lcd_;
  Controller* controller_;
  SensorManager* sensors_;
  TimeManager* rtc_;

  MenuRoot current_root_;
  MenuItem current_item_;
  bool in_submenu_;
  int submenu_index_;
  int submenu_count_;

  // Переменные для кнопок
  unsigned long last_btn_check_;
  unsigned long last_btn_action_;  // Таймер повтора для UP/DOWN
  unsigned long menu_btn_timer_;
  bool menu_btn_pressed_;

  // Временные сообщения на экране
  unsigned long message_timer_;
  const char* temp_message_;

  // Таймер подсветки
  unsigned long last_activity_time_;
  bool backlight_on_;

  // Оптимизация вывода
  ScreenBuffer screen_;
  char last_lines_[2][17];
  bool needs_redraw_;
  void Flush();

  void HandleButtons();
  void DrawPage();
  void DrawRootPage();
  void DrawSubPage();
  void UpdateBacklight();

  // Навигация
  void NextRoot();
  void NextItem();
  void PrevItem();
  void SetDefaultItemForRoot();
  void UpdateSubmenuIndex();

  // Вспомогательные методы отрисовки
  void DrawHomeScreen();
  void DrawStatusIn();
  void DrawStatusOut();
  void DrawTargets();
  void DrawManualModes();
  void DrawCalibPage(const char* label, float value, bool is_temp);
  void DrawStats();
  void DrawErrorLog();

  const MenuItemDef* FindItemDef(MenuItem id);
  const MenuRootDef* FindRootDef(MenuRoot id);

 public:
  static void HandleDrawStatusIn(DisplayUI* ui);
  static void HandleDrawStatusOut(DisplayUI* ui);
  static void HandleDrawTargets(DisplayUI* ui);
  static void HandleDrawManualModes(DisplayUI* ui);
  static void HandleDrawStats(DisplayUI* ui);
  static void HandleDrawErrorLog(DisplayUI* ui);
  static void HandleDrawCalib(DisplayUI* ui);

  static void HandleUpPrevItem(DisplayUI* ui);
  static void HandleDownNextItem(DisplayUI* ui);
  static void HandleUpTargets(DisplayUI* ui);
  static void HandleDownTargets(DisplayUI* ui);
  static void HandleUpManual(DisplayUI* ui);
  static void HandleDownManual(DisplayUI* ui);
  static void HandleMenuManual(DisplayUI* ui);
  static void HandleLongMenuStats(DisplayUI* ui);
  static void HandleUpError(DisplayUI* ui);
  static void HandleUpCalib(DisplayUI* ui);
  static void HandleDownCalib(DisplayUI* ui);

 public:
  DisplayUI(Controller* c, SensorManager* s, TimeManager* t);
  void Init();
  void Reinit();  // Повторная инициализация LCD после сбоя I2C
  void Update();  // Вызывается в основном loop()

  bool IsBacklightOn() const { return backlight_on_; }
};

#endif
