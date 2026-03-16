#include "ButtonEngine.h"

ButtonEngine::ButtonEngine(uint8_t pin_up, uint8_t pin_down, uint8_t pin_menu)
    : pin_up_(pin_up),
      pin_down_(pin_down),
      pin_menu_(pin_menu),
      last_check_(0),
      last_action_(0),
      menu_timer_(0),
      menu_pressed_(false) {}

void ButtonEngine::Init() {
  pinMode(pin_up_, INPUT_PULLUP);
  pinMode(pin_down_, INPUT_PULLUP);
  pinMode(pin_menu_, INPUT_PULLUP);
}

bool ButtonEngine::AnyPressed() const {
  return !digitalRead(pin_up_) || !digitalRead(pin_down_) || !digitalRead(pin_menu_);
}

ButtonEvent ButtonEngine::Poll() {
  if (millis() - last_check_ < kDebounceMs) return ButtonEvent::kNone;
  last_check_ = millis();

  bool up = !digitalRead(pin_up_);
  bool down = !digitalRead(pin_down_);
  bool menu = !digitalRead(pin_menu_);

  // Логика кнопки MENU
  if (menu) {
    if (!menu_pressed_) {
      menu_pressed_ = true;
      menu_timer_ = millis();
    }
  } else {
    if (menu_pressed_) {
      unsigned long duration = millis() - menu_timer_;
      menu_pressed_ = false;
      if (duration >= kLongPressMs) return ButtonEvent::kMenuLong;
      return ButtonEvent::kMenu;
    }
  }

  // Логика UP/DOWN (с автоповтором)
  if ((up || down) && !menu_pressed_) {
    if (millis() - last_action_ >= kRepeatMs) {
      last_action_ = millis();
      return up ? ButtonEvent::kUp : ButtonEvent::kDown;
    }
  }

  return ButtonEvent::kNone;
}
