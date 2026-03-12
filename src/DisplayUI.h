#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Controller.h"
#include "SensorManager.h"
#include "TimeManager.h"

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

class DisplayUI {
 private:
  LiquidCrystal_I2C lcd_;
  Controller* controller_;
  SensorManager* sensors_;
  TimeManager* rtc_;

  MenuRoot current_root_;
  MenuItem current_item_;
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
  DisplayUI(Controller* c, SensorManager* s, TimeManager* t);
  void Init();
  void Reinit();  // Повторная инициализация LCD после сбоя I2C
  void Update();  // Вызывается в основном loop()

  bool IsBacklightOn() const { return backlight_on_; }
};

#endif