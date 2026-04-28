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
  static const uint32_t kDeferredSaveDelay = 5000UL;

  static_assert(kSlotsCount * kSlotSize <= kEepromSize, "EEPROM: slots overflow kEepromSize");

  uint16_t CalculateCrc(const PersistentData& data);
  int FindActiveSlot();

  // Состояние отложенной записи
  PersistentData pending_data_;
  bool needs_save_;
  unsigned long last_change_time_;

 public:
  AppEEPROM();
  void Load(PersistentData& data);
  void Save(const PersistentData& data);

  // Новые методы для инкапсуляции логики планирования
  void ScheduleSave(const PersistentData& data, bool immediate = false);
  void Update();  // Вызывается из главного цикла
};

#endif
