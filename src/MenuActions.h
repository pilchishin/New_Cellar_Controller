#ifndef MENU_ACTIONS_H
#define MENU_ACTIONS_H

#include <Arduino.h>

enum class MenuItemID : uint8_t;
class DisplayUI;

/**
 * @brief Класс, содержащий логику действий при взаимодействии с меню.
 * Вынесен из DisplayUI для улучшения разделения ответственности.
 */
class MenuActions {
 public:
  // Обработчики отрисовки
  static void HandleDrawStatus(DisplayUI* ui);
  static void HandleDrawManualModes(DisplayUI* ui);
  static void HandleDrawStats(DisplayUI* ui);
  static void HandleDrawErrorLog(DisplayUI* ui);
  static void HandleDrawValue(DisplayUI* ui);

  // Обработчики кнопок внутри подменю
  static void HandlePrevItem(DisplayUI* ui);
  static void HandleNextItem(DisplayUI* ui);
  static void HandleUpValue(DisplayUI* ui);
  static void HandleDownValue(DisplayUI* ui);
  static void HandleUpManual(DisplayUI* ui);
  static void HandleDownManual(DisplayUI* ui);
  static void HandleMenuManual(DisplayUI* ui);
  static void HandleLongMenuStats(DisplayUI* ui);
  static void HandleUpError(DisplayUI* ui);

  // Вспомогательные методы
  static void AdjustValue(DisplayUI* ui, float delta);
  static void ApplyValueChange(DisplayUI* ui, MenuItemID id, float val);

  // Обработчики кнопок на уровне корневых разделов
  static void HandlePrevRoot(DisplayUI* ui);
  static void HandleNextRoot(DisplayUI* ui);
  static void HandleEnterSubmenu(DisplayUI* ui);
  static void HandleExitSubmenu(DisplayUI* ui);
};

#endif
