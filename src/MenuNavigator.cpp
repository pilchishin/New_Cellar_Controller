/**
 * @file MenuNavigator.cpp
 * @brief Реализация логики навигации по меню.
 */

#include "MenuNavigator.h"

/**
 * @brief Инициализация навигатора.
 * По умолчанию устанавливается 0-я корневая страница (обычно это экран "HOME").
 */
MenuNavigator::MenuNavigator() : root_index_(0), item_index_(0), in_submenu_(false) {}

/**
 * @brief Переход к следующей странице корневого меню.
 *
 * При переключении страниц основного меню:
 * 1. Индекс страницы инкрементируется циклически.
 * 2. Режим подменю принудительно выключается.
 * 3. Внутренний индекс элемента сбрасывается в 0.
 *
 * @param table_size Количество доступных страниц в ROOT_PAGES.
 */
void MenuNavigator::NextRoot(uint8_t table_size) {
  if (table_size > 0) {
    root_index_ = (root_index_ + 1) % table_size;
    item_index_ = 0;
    in_submenu_ = false;
  }
}

/**
 * @brief Переход к предыдущей странице корневого меню.
 *
 * Реализует циклическую навигацию в обратном направлении.
 * Сбрасывает состояние подменю при каждом переключении страницы.
 *
 * @param table_size Количество доступных страниц в ROOT_PAGES.
 */
void MenuNavigator::PrevRoot(uint8_t table_size) {
  if (table_size > 0) {
    root_index_ = (root_index_ + table_size - 1) % table_size;
    item_index_ = 0;
    in_submenu_ = false;
  }
}

/**
 * @brief Переход к следующему пункту или разряду внутри подменю.
 *
 * Используется, например, для циклического переключения между часами и минутами
 * или между редактируемыми цифрами значения.
 *
 * @param item_count Количество активных зон взаимодействия в подменю.
 */
void MenuNavigator::NextItem(uint8_t item_count) {
  if (item_count > 0) {
    item_index_ = (item_index_ + 1) % item_count;
  }
}

/**
 * @brief Переход к предыдущему пункту или разряду внутри подменю.
 *
 * @param item_count Количество активных зон взаимодействия в подменю.
 */
void MenuNavigator::PrevItem(uint8_t item_count) {
  if (item_count > 0) {
    item_index_ = (item_index_ + item_count - 1) % item_count;
  }
}

/**
 * @brief Вход в режим редактирования/подменю.
 *
 * Проверяет, не является ли текущая страница главной (HOME), так как на ней
 * нет параметров для редактирования. При входе сбрасывает item_index_ в 0.
 */
void MenuNavigator::EnterSubmenu() {
  if (root_index_ != 0) { // Экран HOME не имеет подменю
    in_submenu_ = true;
    item_index_ = 0;
  }
}

/**
 * @brief Выход из режима подменю.
 * Возвращает управление в основной список страниц меню.
 */
void MenuNavigator::ExitSubmenu() {
  in_submenu_ = false;
}
