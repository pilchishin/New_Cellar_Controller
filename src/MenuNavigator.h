#ifndef MENU_NAVIGATOR_H
#define MENU_NAVIGATOR_H

#include <Arduino.h>

/**
 * @brief Класс управления состоянием навигации по меню.
 * Инкапсулирует индексы и логику переходов.
 */
class MenuNavigator {
 public:
  MenuNavigator();

  uint8_t GetRootIndex() const { return root_index_; }
  uint8_t GetItemIndex() const { return item_index_; }
  bool InSubmenu() const { return in_submenu_; }

  void NextRoot(uint8_t table_size);
  void PrevRoot(uint8_t table_size);
  void NextItem(uint8_t item_count);
  void PrevItem(uint8_t item_count);
  void EnterSubmenu();
  void ExitSubmenu();

  // Методы для прямой манипуляции в особых случаях
  void SetItemIndex(uint8_t index) { item_index_ = index; }

 private:
  uint8_t root_index_;
  uint8_t item_index_;
  bool in_submenu_;
};

#endif
