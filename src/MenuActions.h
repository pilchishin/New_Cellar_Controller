#ifndef MENU_ACTIONS_H
#define MENU_ACTIONS_H

#include <Arduino.h>

// Предварительные объявления для минимизации заголовочных связей
enum class ActionID : uint8_t;
enum class MenuItemID : uint8_t;
class DisplayUI;

/**
 * @class MenuActions
 * @brief Статический класс-исполнитель действий меню.
 * Инкапсулирует бизнес-логику реакций на события пользовательского интерфейса.
 * Позволяет отделить навигацию и отрисовку от непосредственного изменения
 * параметров системы и управления оборудованием.
 */
class MenuActions {
 public:
  /**
   * @brief Главный маршрутизатор действий.
   * На основе ActionID вызывает соответствующий метод логики.
   * @param ui Указатель на экземпляр интерфейса.
   * @param action Идентификатор действия для выполнения.
   */
  static void Execute(DisplayUI* ui, ActionID action);

  /**
   * @brief Обработчик команды увеличения значения.
   * Читает конфигурацию шага из PROGMEM и применяет дельту.
   */
  static void HandleUpValue(DisplayUI* ui);

  /**
   * @brief Обработчик команды уменьшения значения.
   */
  static void HandleDownValue(DisplayUI* ui);

  /**
   * @brief Обработчик ручного пуска оборудования (вентилятор/озон).
   */
  static void HandleMenuManual(DisplayUI* ui);

  /**
   * @brief Обработчик подтверждения сброса статистики.
   */
  static void HandleLongMenuStats(DisplayUI* ui);

  /**
   * @brief Универсальный метод изменения значения с проверкой границ.
   * @param ui Указатель на интерфейс.
   * @param delta Величина изменения.
   */
  static void AdjustValue(DisplayUI* ui, float delta);

  /**
   * @brief Синхронизация измененного значения с контроллером и моделью UI.
   * @param ui Указатель на интерфейс.
   * @param id Идентификатор элемента меню.
   * @param val Новое значение.
   */
  static void ApplyValueChange(DisplayUI* ui, MenuItemID id, float val);
};

#endif
