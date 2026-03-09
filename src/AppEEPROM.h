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
  static const uint16_t kEepromSize = 1024;   // Для ATmega328P
  static const uint8_t kSlotsCount = 10;      // Количество слотов для wear leveling
  static const uint16_t kSlotSize = sizeof(PersistentData);

  uint16_t CalculateCrc(const PersistentData& data);
  int FindActiveSlot();

 public:
  AppEEPROM();
  void Load(PersistentData& data);
  void Save(const PersistentData& data);
};

#endif
