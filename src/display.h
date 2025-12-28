#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"
#include <LiquidCrystal.h>

class DisplayManager {
public:
    DisplayManager();
    void init();
    void update();
    
    void clear();
    void setCursor(int col, int row);
    void print(const char* text);
    void printAt(int col, int row, const char* text);
    
    void displayMainMenu();
    void displayStatusScreen();
    void displaySettingsScreen();
    void displayCalibrationScreen();
    void displayOzoneScreen();
    
    void showTemperature(float temp);
    void showHumidity(float humidity);
    void showOzoneLevel(float ozone);
    void showTime(const char* timeStr);
    
    void backlightOn();
    void backlightOff();
    bool isBacklightOn();
    
    void refresh();
    
private:
    LiquidCrystal* lcd;
    bool backlightStatus;
};

#endif // DISPLAY_H