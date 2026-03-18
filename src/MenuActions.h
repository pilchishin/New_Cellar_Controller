#ifndef MENU_ACTIONS_H
#define MENU_ACTIONS_H

#include <Arduino.h>

enum class ActionID : uint8_t;
enum class MenuItemID : uint8_t;
class DisplayUI;

/**
 * @brief Класс, содержащий логику действий при взаимодействии с меню.
 * Вынесен из DisplayUI для улучшения разделения ответственности.
 */
class MenuActions {
 public:
  // Центральный исполнитель действий
  static void Execute(DisplayUI* ui, ActionID action);

  // Обработчики кнопок внутри подменю
  static void HandleUpValue(DisplayUI* ui);
  static void HandleDownValue(DisplayUI* ui);
  static void HandleMenuManual(DisplayUI* ui);
  static void HandleLongMenuStats(DisplayUI* ui);

  // Вспомогательные методы
  static void AdjustValue(DisplayUI* ui, float delta);
  static void ApplyValueChange(DisplayUI* ui, MenuItemID id, float val);
};

#endif
