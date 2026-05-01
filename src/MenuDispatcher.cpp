/**
 * @file MenuDispatcher.cpp
 * @brief Реализация логики распределения событий меню.
 */

#include "MenuDispatcher.h"
#include "DisplayUI.h"
#include "MenuActions.h"

/**
 * @brief Маршрутизирует события кнопок к соответствующим действиям меню.
 *
 * Логика работы:
 * 1. Игнорирует пустые события (kNone).
 * 2. Проверяет текущее состояние навигации (находится ли пользователь в подменю редактирования).
 * 3. Если пользователь в корневом меню:
 *    - Получает определение текущей страницы меню (MenuRootDef).
 *    - Сопоставляет событие кнопки (Up/Down/Menu/LongMenu) с действием, прописанным в определении страницы.
 * 4. Если пользователь в подменю (редактирование параметра):
 *    - Получает определение текущего элемента настройки (MenuItemDef).
 *    - Сопоставляет событие кнопки с действием, специфичным для этого элемента.
 * 5. Если в результате сопоставления определено конкретное действие (ActionID), оно выполняется через MenuActions::Execute.
 *
 * @param ui Указатель на экземпляр DisplayUI для доступа к навигации и данным.
 * @param event Тип произошедшего события кнопки.
 */
void MenuDispatcher::Dispatch(DisplayUI* ui, ButtonEvent event) {
  // Игнорируем отсутствие события
  if (event == ButtonEvent::kNone) return;

  ActionID action = ActionID::kNone;

  // Проверяем, не находимся ли мы в режиме редактирования значения (подменю)
  if (!ui->nav_.InSubmenu()) {
    // Логика для основного меню навигации
    const MenuRootDef* root = ui->GetCurrentRootDef();
    if (!root) return;

    // Мапинг кнопок на действия корневого меню
    switch (event) {
      case ButtonEvent::kUp:       action = root->up_action; break;
      case ButtonEvent::kDown:     action = root->down_action; break;
      case ButtonEvent::kMenu:     action = root->menu_action; break;
      case ButtonEvent::kMenuLong: action = root->long_menu_action; break;
      default: break;
    }
  } else {
    // Логика для режима редактирования параметра (внутри подменю)
    const MenuItemDef* item = ui->GetCurrentItemDef();
    if (!item) return;

    // Мапинг кнопок на действия конкретного элемента настройки
    switch (event) {
      case ButtonEvent::kUp:       action = item->up_action; break;
      case ButtonEvent::kDown:     action = item->down_action; break;
      case ButtonEvent::kMenu:     action = item->menu_action; break;
      case ButtonEvent::kMenuLong: action = item->long_menu_action; break;
      default: break;
    }
  }

  // Если действие определено, выполняем его
  if (action != ActionID::kNone) {
    MenuActions::Execute(ui, action);
  }
}
