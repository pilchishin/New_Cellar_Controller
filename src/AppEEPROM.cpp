#include "AppEEPROM.h"
#include <EEPROM.h>
#include "config.h"

AppEEPROM::AppEEPROM() : needs_save_(false), last_change_time_(0) {}

uint16_t AppEEPROM::CalculateCrc(const PersistentData& data) {
  uint16_t crc = 0xFFFF;
  const uint8_t* bytes = (const uint8_t*)&data;
  // Считаем CRC для всей структуры, кроме самого поля crc (последние 2 байта)
  for (uint16_t i = 0; i < sizeof(PersistentData) - 2; i++) {
    crc ^= bytes[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 1)
        crc = (crc >> 1) ^ 0xA001;
      else
        crc >>= 1;
    }
  }
  return crc;
}

/**
 * @brief Поиск последнего валидного слота с данными.
 */
int AppEEPROM::FindActiveSlot() {
  for (int i = kSlotsCount - 1; i >= 0; i--) {
    PersistentData temp;
    EEPROM.get(i * kSlotSize, temp);
    if (temp.crc == CalculateCrc(temp)) {
      return i;
    }
  }
  return -1;  // Ни один слот не прошел проверку CRC
}

void AppEEPROM::Load(PersistentData& data) {
  int slot = FindActiveSlot();
  if (slot != -1) {
    EEPROM.get(slot * kSlotSize, data);
#ifdef DEBUG
    Serial.print(F("EEPROM: Loaded from slot "));
    Serial.println(slot);
#endif
  } else {
    // Если данных нет, инициализируем нулями (или дефолтами)
    memset(&data, 0, sizeof(PersistentData));
    data.targetTemp = kDefaultTargetTemp;
    data.targetRh = kDefaultTargetRh;
#ifdef DEBUG
    Serial.println(F("EEPROM: No valid data found. Defaults loaded."));
#endif
  }
}

/**
 * @brief Сохранение данных с использованием алгоритма выравнивания износа.
 */
void AppEEPROM::Save(const PersistentData& data) {
  PersistentData copy = data;
  copy.crc = CalculateCrc(copy);  // Вычисление CRC для обеспечения целостности

  int current_slot = FindActiveSlot();
  int next_slot = (current_slot + 1) % kSlotsCount;

  // Записываем новые данные в следующий по порядку слот
  EEPROM.put(next_slot * kSlotSize, copy);

  // Стираем CRC в старом слоте
  if (current_slot != -1 && current_slot != next_slot) {
    uint16_t invalid_crc = 0;
    EEPROM.put(current_slot * kSlotSize + offsetof(PersistentData, crc),
               invalid_crc);
  }

#ifdef DEBUG
  Serial.print(F("EEPROM: Saved to slot "));
  Serial.println(next_slot);
#endif
}

void AppEEPROM::ScheduleSave(const PersistentData& data, bool immediate) {
  pending_data_ = data;
  needs_save_ = true;
  if (immediate) {
    last_change_time_ = millis() - kDeferredSaveDelay;
  } else {
    last_change_time_ = millis();
  }
}

void AppEEPROM::Update() {
  if (needs_save_ && (millis() - last_change_time_ >= kDeferredSaveDelay)) {
    Save(pending_data_);
    needs_save_ = false;
  }
}
