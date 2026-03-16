#include "MenuNavigator.h"

MenuNavigator::MenuNavigator() : root_index_(0), item_index_(0), in_submenu_(false) {}

void MenuNavigator::NextRoot(uint8_t table_size) {
  if (table_size > 0) {
    root_index_ = (root_index_ + 1) % table_size;
    item_index_ = 0;
    in_submenu_ = false;
  }
}

void MenuNavigator::PrevRoot(uint8_t table_size) {
  if (table_size > 0) {
    root_index_ = (root_index_ + table_size - 1) % table_size;
    item_index_ = 0;
    in_submenu_ = false;
  }
}

void MenuNavigator::NextItem(uint8_t item_count) {
  if (item_count > 0) {
    item_index_ = (item_index_ + 1) % item_count;
  }
}

void MenuNavigator::PrevItem(uint8_t item_count) {
  if (item_count > 0) {
    item_index_ = (item_index_ + item_count - 1) % item_count;
  }
}

void MenuNavigator::EnterSubmenu() {
  if (root_index_ != 0) { // HOME не имеет подменю
    in_submenu_ = true;
    item_index_ = 0;
  }
}

void MenuNavigator::ExitSubmenu() {
  in_submenu_ = false;
}
