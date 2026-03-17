#include "MenuDispatcher.h"
#include "DisplayUI.h"

void MenuDispatcher::Dispatch(DisplayUI* ui, ButtonEvent event) {
  if (event == ButtonEvent::kNone) return;

  if (!ui->nav_.InSubmenu()) {
    const MenuRootDef* root = ui->GetCurrentRootDef();
    if (!root) return;

    switch (event) {
      case ButtonEvent::kUp:   if (root->on_up) root->on_up(ui); break;
      case ButtonEvent::kDown: if (root->on_down) root->on_down(ui); break;
      case ButtonEvent::kMenu: if (root->on_menu) root->on_menu(ui); break;
      case ButtonEvent::kMenuLong: if (root->on_long_menu) root->on_long_menu(ui); break;
      default: break;
    }
  } else {
    const MenuItemDef* item = ui->GetCurrentItemDef();
    if (!item) return;

    switch (event) {
      case ButtonEvent::kUp:   if (item->on_up) item->on_up(ui); break;
      case ButtonEvent::kDown: if (item->on_down) item->on_down(ui); break;
      case ButtonEvent::kMenu: if (item->on_menu) item->on_menu(ui); break;
      case ButtonEvent::kMenuLong: if (item->on_long_menu) item->on_long_menu(ui); break;
      default: break;
    }
  }
}
