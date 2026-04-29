#ifndef EEPROM_H
#define EEPROM_H
#include <stdint.h>
#include <string.h>

class EEPROMMock {
public:
    uint8_t data[1024];
    EEPROMMock() { memset(data, 0xFF, sizeof(data)); }
    uint8_t read(int addr) { return data[addr % 1024]; }
    void write(int addr, uint8_t val) { data[addr % 1024] = val; }
    void update(int addr, uint8_t val) { data[addr % 1024] = val; }

    template<typename T> T& get(int addr, T& t) {
        memcpy(&t, &data[addr % 1024], sizeof(T));
        return t;
    }
    template<typename T> const T& put(int addr, const T& t) {
        memcpy(&data[addr % 1024], &t, sizeof(T));
        return t;
    }
    uint16_t length() { return 1024; }
};

extern EEPROMMock EEPROM;

#endif
