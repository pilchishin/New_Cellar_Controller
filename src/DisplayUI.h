#ifndef DISPLAY_UI_H
#define DISPLAY_UI_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "Controller.h"
#include "SensorManager.h"
#include "TimeManager.h"

// Состояния меню
enum class MenuPage {
    HOME_SCREEN,    // Главная: T/H, цели и статусы реле
    STATUS_IN,      // Датчики внутри (детально: AH, точка росы)
    STATUS_OUT,     // Датчики снаружи (детально: AH)
    SET_TEMP,       // Установка целевой T
    SET_HUM,        // Установка целевой RH
    MANUAL_MODES,   // Ручной запуск FAN/OZONE
    ERROR_LOG,      // Просмотр и сброс ошибок
    STATS,          // Статистика работы
    CALIB_BME_T,    // Калибровка BME T
    CALIB_BME_H,    // Калибровка BME H
    CALIB_HTU_T,    // Калибровка HTU T
    CALIB_HTU_H,    // Калибровка HTU H
    CALIB_DS_T      // Калибровка DS T
};

class DisplayUI {
 private:
  LiquidCrystal_I2C lcd_;
  Controller* controller_;
  SensorManager* sensors_;
  TimeManager* rtc_;

  MenuPage current_page_;

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
  void UpdateBacklight();

  // Вспомогательные методы отрисовки
  void DrawHomeScreen();
  void DrawStatusIn();
  void DrawStatusOut();
  void DrawSetTemp();
  void DrawSetHum();
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