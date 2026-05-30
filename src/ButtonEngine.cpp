#include "ButtonEngine.h"

ButtonEngine::ButtonEngine(uint8_t pin_up, uint8_t pin_down, uint8_t pin_menu)
    : pin_up_(pin_up),
      pin_down_(pin_down),
      pin_menu_(pin_menu),
      last_check_(0),
      last_action_(0),
      menu_timer_(0),
      menu_pressed_(false) {}

/**
 * @brief Инициализация периферии.
 * Кнопки подключаются между пином и землей (GND).
 */
void ButtonEngine::Init() {
  pinMode(pin_up_, INPUT_PULLUP);
  pinMode(pin_down_, INPUT_PULLUP);
  pinMode(pin_menu_, INPUT_PULLUP);
}

/**
 * @brief Проверка физического состояния кнопок (инвертировано, т.к. PULLUP).
 */
bool ButtonEngine::AnyPressed() const {
  return !digitalRead(pin_up_) || !digitalRead(pin_down_) || !digitalRead(pin_menu_);
}

/**
 * @brief Основной цикл обработки кнопок.
 * Реализует неблокирующий опрос с программной фильтрацией дребезга контактов.
 */
ButtonEvent ButtonEngine::Poll() {
  // Программный антидребезг: не опрашиваем чаще, чем задано в kDebounceMs
  if (millis() - last_check_ < kDebounceMs) return ButtonEvent::kNone;
  last_check_ = millis();

  // Считываем физическое состояние (LOW = нажато из-за подтяжки к VCC)
  bool up = !digitalRead(pin_up_);
  bool down = !digitalRead(pin_down_);
  bool menu = !digitalRead(pin_menu_);

  // --- Логика кнопки MENU ---
  // Определяется по моменту отпускания: короткий клик или длинное нажатие.
  if (menu) {
    if (!menu_pressed_) {
      // Кнопка только что нажата
      menu_pressed_ = true;
      menu_timer_ = millis();
    }
    // Защита от залипания (stuck button detection)
    if (millis() - menu_timer_ >= kButtonStuckTimeout) {
      menu_pressed_ = false;
      return ButtonEvent::kNone;
    }
  } else {
    if (menu_pressed_) {
      // Кнопка только что отпущена — вычисляем длительность удержания
      unsigned long duration = millis() - menu_timer_;
      menu_pressed_ = false;
      if (duration >= kLongPressMs) return ButtonEvent::kMenuLong;
      return ButtonEvent::kMenu;
    }
  }

  // --- Логика кнопок UP/DOWN ---
  // Реализован автоповтор: если кнопку держать, события генерируются периодически.
  if ((up || down) && !menu_pressed_) {
    if (continuous_press_start_ == 0) {
      continuous_press_start_ = millis();
    }
    // Защита от залипания
    if (millis() - continuous_press_start_ >= kButtonStuckTimeout) {
      return ButtonEvent::kNone; // Игнорируем — кнопка похоже залипла
    }
    if (millis() - last_action_ >= kRepeatMs) {
      last_action_ = millis();
      return up ? ButtonEvent::kUp : ButtonEvent::kDown;
    }
  } else {
    continuous_press_start_ = 0; // Сброс при отпускании или переключении на MENU
  }

  return ButtonEvent::kNone;
}
