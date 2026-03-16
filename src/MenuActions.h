#ifndef MENU_ACTIONS_H
#define MENU_ACTIONS_H

#include <Arduino.h>

class DisplayUI;

/**
 * @brief Класс, содержащий логику действий при взаимодействии с меню.
 * Вынесен из DisplayUI для улучшения разделения ответственности.
 */
class MenuActions {
 public:
  // Обработчики отрисовки
  static void HandleDrawStatusIn(DisplayUI* ui);
  static void HandleDrawStatusOut(DisplayUI* ui);
  static void HandleDrawTargets(DisplayUI* ui);
  static void HandleDrawManualModes(DisplayUI* ui);
  static void HandleDrawStats(DisplayUI* ui);
  static void HandleDrawErrorLog(DisplayUI* ui);
  static void HandleDrawCalib(DisplayUI* ui);

  // Обработчики кнопок внутри подменю
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

  // Обработчики кнопок на уровне корневых разделов
  static void HandlePrevRoot(DisplayUI* ui);
  static void HandleNextRoot(DisplayUI* ui);
  static void HandleEnterSubmenu(DisplayUI* ui);
  static void HandleExitSubmenu(DisplayUI* ui);
};

#endif
