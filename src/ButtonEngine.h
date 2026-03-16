#ifndef BUTTON_ENGINE_H
#define BUTTON_ENGINE_H

#include <Arduino.h>

/**
 * @brief Типы событий кнопок.
 */
enum class ButtonEvent {
  kNone,
  kUp,
  kDown,
  kMenu,
  kMenuLong
};

/**
 * @brief Двигатель обработки кнопок.
 * Реализует антидребезг, автоповтор для UP/DOWN и долгое нажатие для MENU.
 */
class ButtonEngine {
 public:
  ButtonEngine(uint8_t pin_up, uint8_t pin_down, uint8_t pin_menu);

  void Init();

  /**
   * @brief Опрос состояния кнопок.
   * @return Событие кнопки или kNone, если событий нет.
   */
  ButtonEvent Poll();

  /**
   * @brief Проверка, была ли нажата любая кнопка (для управления подсветкой).
   */
  bool AnyPressed() const;

 private:
  uint8_t pin_up_, pin_down_, pin_menu_;

  unsigned long last_check_;
  unsigned long last_action_;
  unsigned long menu_timer_;
  bool menu_pressed_;

  const unsigned long kDebounceMs = 50;
  const unsigned long kRepeatMs = 150;
  const unsigned long kLongPressMs = 600;
};

#endif
