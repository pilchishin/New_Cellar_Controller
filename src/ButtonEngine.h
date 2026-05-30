#ifndef BUTTON_ENGINE_H
#define BUTTON_ENGINE_H

#include <Arduino.h>
#include "config.h"

/**
 * @enum ButtonEvent
 * @brief Перечисление типов событий, генерируемых кнопками.
 */
enum class ButtonEvent {
  kNone,      ///< Событий нет
  kUp,        ///< Нажата кнопка ВВЕРХ (или сработал автоповтор)
  kDown,      ///< Нажата кнопка ВНИЗ (или сработал автоповтор)
  kMenu,      ///< Короткое нажатие кнопки МЕНЮ
  kMenuLong   ///< Длительное нажатие кнопки МЕНЮ (> 600 мс)
};

/**
 * @class ButtonEngine
 * @brief Двигатель (обработчик) кнопок управления.
 * Реализует низкоуровневую логику: программный антидребезг (debounce),
 * автоповтор для кнопок навигации (UP/DOWN) и распознавание длительного нажатия
 * для кнопки выбора (MENU). Использует подтяжку INPUT_PULLUP.
 */
class ButtonEngine {
 public:
  /**
   * @brief Конструктор обработчика.
   * @param pin_up Пин кнопки ВВЕРХ.
   * @param pin_down Пин кнопки ВНИЗ.
   * @param pin_menu Пин кнопки МЕНЮ.
   */
  ButtonEngine(uint8_t pin_up, uint8_t pin_down, uint8_t pin_menu);

  /**
   * @brief Настройка пинов в режим ввода с подтяжкой.
   */
  void Init();

  /**
   * @brief Опрос состояния кнопок и генерация событий.
   * Должен вызываться регулярно в главном цикле.
   * @return Событие ButtonEvent или kNone, если состояние не изменилось.
   */
  ButtonEvent Poll();

  /**
   * @brief Проверка физического нажатия любой из кнопок.
   * Полезно для сброса таймера подсветки экрана.
   * @return true, если в данный момент зажата хотя бы одна кнопка.
   */
  bool AnyPressed() const;

 private:
  uint8_t pin_up_, pin_down_, pin_menu_; ///< Номера пинов для кнопок

  unsigned long last_check_;   ///< Время последней проверки (для антидребезга)
  unsigned long last_action_;  ///< Время последнего сгенерированного события UP/DOWN (для автоповтора)
  unsigned long menu_timer_;   ///< Время начала нажатия кнопки MENU
  bool menu_pressed_;          ///< Флаг текущего зажатого состояния кнопки MENU
  unsigned long continuous_press_start_ = 0; ///< Время начала непрерывного удержания любой кнопки
};

#endif
