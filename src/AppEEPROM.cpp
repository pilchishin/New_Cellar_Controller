#include "AppEEPROM.h"
#include <EEPROM.h>

AppEEPROM::AppEEPROM() {}

uint16_t AppEEPROM::calculateCRC(const PersistentData& data) {
    uint16_t crc = 0xFFFF;
    const uint8_t* bytes = (const uint8_t*)&data;
    // Считаем CRC для всей структуры, кроме самого поля crc (последние 2 байта)
    for (uint16_t i = 0; i < sizeof(PersistentData) - 2; i++) {
        crc ^= bytes[i];
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 1) crc = (crc >> 1) ^ 0xA001;
            else crc >>= 1;
        }
    }
    return crc;
}

int AppEEPROM::findActiveSlot() {
    for (int i = 0; i < SLOTS_COUNT; i++) {
        PersistentData temp;
        EEPROM.get(i * SLOT_SIZE, temp);
        if (temp.crc == calculateCRC(temp)) {
            return i;
        }
    }
    return -1; // Валидных данных нет
}

void AppEEPROM::load(PersistentData& data) {
    int slot = findActiveSlot();
    if (slot != -1) {
        EEPROM.get(slot * SLOT_SIZE, data);
        #ifdef DEBUG
        Serial.print(F("EEPROM: Loaded from slot ")); Serial.println(slot);
        #endif
    } else {
        // Если данных нет, инициализируем нулями (или дефолтами)
        memset(&data, 0, sizeof(PersistentData));
        data.targetTemp = 4.0f;
        data.targetRh = 85.0f;
        #ifdef DEBUG
        Serial.println(F("EEPROM: No valid data found. Defaults loaded."));
        #endif
    }
}

void AppEEPROM::save(const PersistentData& data) {
    PersistentData copy = data;
    copy.crc = calculateCRC(copy);

    int currentSlot = findActiveSlot();
    int nextSlot = (currentSlot + 1) % SLOTS_COUNT;

    // Записываем в следующий слот (Wear Leveling)
    EEPROM.put(nextSlot * SLOT_SIZE, copy);

    // Стираем CRC в старом слоте, чтобы пометить его неактивным (опционально для ускорения поиска)
    if (currentSlot != -1 && currentSlot != nextSlot) {
        uint16_t invalidCrc = 0;
        EEPROM.put(currentSlot * SLOT_SIZE + offsetof(PersistentData, crc), invalidCrc);
    }

    #ifdef DEBUG
    Serial.print(F("EEPROM: Saved to slot ")); Serial.println(nextSlot);
    #endif
}
