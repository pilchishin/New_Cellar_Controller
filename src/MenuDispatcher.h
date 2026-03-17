#ifndef MENU_DISPATCHER_H
#define MENU_DISPATCHER_H

#include <Arduino.h>
#include "ButtonEngine.h"

class DisplayUI;

/**
 * @brief Класс, отвечающий за маршрутизацию событий кнопок к соответствующим обработчикам меню.
 */
class MenuDispatcher {
 public:
  /**
   * @brief Перенаправляет событие кнопки активному элементу меню.
   * @param ui Указатель на экземпляр интерфейса.
   * @param event Тип события кнопки.
   */
  void Dispatch(DisplayUI* ui, ButtonEvent event);
};

#endif
