#ifndef APP_EEPROM_H
#define APP_EEPROM_H

#include <Arduino.h>
#include "Types.h"

// Структура всех сохраняемых данных
struct PersistentData {
    float targetTemp;
    float targetRh;
    CalibrationData calibration;
    SystemStatistics stats;
    uint16_t crc;
};

class AppEEPROM {
private:
    static const uint16_t EEPROM_SIZE = 1024; // Для ATmega328P
    static const uint8_t SLOTS_COUNT = 10;    // Количество слотов для wear leveling
    static const uint16_t SLOT_SIZE = sizeof(PersistentData);

    uint16_t calculateCRC(const PersistentData& data);
    int findActiveSlot();

public:
    AppEEPROM();
    void load(PersistentData& data);
    void save(const PersistentData& data);
};

#endif
