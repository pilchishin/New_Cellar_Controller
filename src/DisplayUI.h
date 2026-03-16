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

struct MenuItemDef {
  const char* label;
  void (*draw)(DisplayUI* ui);
  void (*on_up)(DisplayUI* ui);
  void (*on_down)(DisplayUI* ui);
  void (*on_menu)(DisplayUI* ui);
  void (*on_long_menu)(DisplayUI* ui);
};

struct MenuRootDef {
  const char* label;
  const MenuItemDef* items;
  uint8_t item_count;
  void (*on_up)(DisplayUI* ui);
  void (*on_down)(DisplayUI* ui);
  void (*on_menu)(DisplayUI* ui);
  void (*on_long_menu)(DisplayUI* ui);
};

class DisplayUI {
 private:
  LiquidCrystal_I2C lcd_;
  Controller* controller_;
  SensorManager* sensors_;
  TimeManager* rtc_;

  uint8_t root_index_;
  uint8_t item_index_;
  bool in_submenu_;

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
  void UpdateBacklight();

  void DrawHeader(const char* label, uint8_t index, uint8_t count);
  void DrawHeader(const __FlashStringHelper* label, uint8_t index, uint8_t count);

  // Вспомогательные методы отрисовки
  void DrawHomeScreen();
  void DrawStatusIn();
  void DrawStatusOut();
  void DrawTargets();
  void DrawManualModes();
  void DrawCalibPage(const char* label, float value, bool is_temp);
  void DrawStats();
  void DrawErrorLog();

 public:
  const MenuRootDef* GetCurrentRootDef() const;
  const MenuItemDef* GetCurrentItemDef() const;
  static void HandleDrawStatusIn(DisplayUI* ui);
  static void HandleDrawStatusOut(DisplayUI* ui);
  static void HandleDrawTargets(DisplayUI* ui);
  static void HandleDrawManualModes(DisplayUI* ui);
  static void HandleDrawStats(DisplayUI* ui);
  static void HandleDrawErrorLog(DisplayUI* ui);
  static void HandleDrawCalib(DisplayUI* ui);

  static void HandlePrevItem(DisplayUI* ui);
  static void HandleNextItem(DisplayUI* ui);
  static void HandleUpTargets(DisplayUI* ui);
  static void HandleDownTargets(DisplayUI* ui);
  static void HandleUpManual(DisplayUI* ui);
  static void HandleDownManual(DisplayUI* ui);
  static void HandleMenuManual(DisplayUI* ui);
  static void HandleLongMenuStats(DisplayUI* ui);
  static void HandleUpError(DisplayUI* ui);
  static void HandleUpCalib(DisplayUI* ui);
  static void HandleDownCalib(DisplayUI* ui);
  static void AdjustCalib(DisplayUI* ui, float delta);

  static void HandlePrevRoot(DisplayUI* ui);
  static void HandleNextRoot(DisplayUI* ui);
  static void HandleEnterSubmenu(DisplayUI* ui);
  static void HandleExitSubmenu(DisplayUI* ui);

  static MenuRootDef current_root_buf_;
  static MenuItemDef current_item_buf_;

 public:
  DisplayUI(Controller* c, SensorManager* s, TimeManager* t);
  void Init();
  void Reinit();  // Повторная инициализация LCD после сбоя I2C
  void Update();  // Вызывается в основном loop()

  bool IsBacklightOn() const { return backlight_on_; }
};

#endif
