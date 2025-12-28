#ifndef MENU_H
#define MENU_H

#include "config.h"

class MenuManager {
public:
    MenuManager();
    void init();
    void update();
    
    void displayMenu();
    void navigateUp();
    void navigateDown();
    void select();
    void back();
    
    void enterSettingsMenu();
    void enterCalibrationMenu();
    void enterStatusMenu();
    void enterOzoneMenu();
    
    bool isMenuActive();
    void activateMenu();
    void deactivateMenu();
    
    void refreshDisplay();
    
private:
    int currentMenuIndex;
    int menuItemCount;
    bool menuActive;
};

#endif // MENU_H