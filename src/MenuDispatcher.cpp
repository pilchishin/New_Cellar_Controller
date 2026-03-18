#include "MenuDispatcher.h"
#include "DisplayUI.h"
#include "MenuActions.h"

void MenuDispatcher::Dispatch(DisplayUI* ui, ButtonEvent event) {
  if (event == ButtonEvent::kNone) return;

  ActionID action = ActionID::kNone;

  if (!ui->nav_.InSubmenu()) {
    const MenuRootDef* root = ui->GetCurrentRootDef();
    if (!root) return;

    switch (event) {
      case ButtonEvent::kUp:       action = root->up_action; break;
      case ButtonEvent::kDown:     action = root->down_action; break;
      case ButtonEvent::kMenu:     action = root->menu_action; break;
      case ButtonEvent::kMenuLong: action = root->long_menu_action; break;
      default: break;
    }
  } else {
    const MenuItemDef* item = ui->GetCurrentItemDef();
    if (!item) return;

    switch (event) {
      case ButtonEvent::kUp:       action = item->up_action; break;
      case ButtonEvent::kDown:     action = item->down_action; break;
      case ButtonEvent::kMenu:     action = item->menu_action; break;
      case ButtonEvent::kMenuLong: action = item->long_menu_action; break;
      default: break;
    }
  }

  if (action != ActionID::kNone) {
    MenuActions::Execute(ui, action);
  }
}
