#ifndef LIQUIDCRYSTAL_I2C_H
#define LIQUIDCRYSTAL_I2C_H
#include <stdint.h>
class LiquidCrystal_I2C {
public:
    LiquidCrystal_I2C(uint8_t addr, uint8_t cols, uint8_t rows) {}
    void init() {}
    void backlight() {}
    void noBacklight() {}
    void clear() {}
    void setCursor(uint8_t col, uint8_t row) {}
    void print(const char* s) {}
    void print(float f, int p = 2) {}
    void print(int i) {}
};
#endif
