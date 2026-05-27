#include "AppEEPROM.h"
#include <EEPROM.h>
#include "config.h"

AppEEPROM::AppEEPROM() : needs_save_(false), last_change_time_(0) {}

/**
 * @brief Реализация CRC16 (алгоритм MODBUS).
 * Используется для проверки целостности данных при чтении из EEPROM.
 * Исключает из расчета последние 2 байта структуры (где хранится сам CRC).
 */
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
 * Выбирает слот с максимальным порядковым номером (seq) среди всех валидных.
 * @return Индекс слота (0..kSlotsCount-1) или -1, если данные повреждены или отсутствуют.
 */
int AppEEPROM::FindActiveSlot() {
  int best_slot = -1;
  uint16_t max_seq = 0;

  for (int i = 0; i < kSlotsCount; i++) {
    PersistentData temp;
    EEPROM.get(i * kSlotSize, temp);
    // Проверка целостности через CRC
    if (temp.crc == CalculateCrc(temp)) {
      // Если это первый валидный слот или его seq больше (с учетом переполнения uint16_t
      // это упрощенная проверка, для 10 слотов достаточно обычного > или >=)
      if (best_slot == -1 || temp.seq >= max_seq) {
        max_seq = temp.seq;
        best_slot = i;
      }
    }
  }
  return best_slot;
}

/**
 * @brief Загрузка параметров системы.
 * Ищет активный слот и считывает данные. Если данных нет (первый запуск),
 * загружает заводские установки из config.h.
 */
void AppEEPROM::Load(PersistentData& data) {
  int slot = FindActiveSlot();
  if (slot != -1) {
    EEPROM.get(slot * kSlotSize, data);
    current_slot_seq_ = data.seq;
#ifdef DEBUG
    Serial.print(F("EEPROM: Loaded from slot "));
    Serial.print(slot);
    Serial.print(F(" with seq "));
    Serial.println(current_slot_seq_);
#endif
  } else {
    // Если данных нет, инициализируем нулями и загружаем значения по умолчанию
    memset(&data, 0, sizeof(PersistentData));
    data.targetTemp = kDefaultTargetTemp;
    data.targetRh = kDefaultTargetRh;
    current_slot_seq_ = 0;
#ifdef DEBUG
    Serial.println(F("EEPROM: No valid data found. Defaults loaded."));
#endif
  }
}

/**
 * @brief Сохранение данных с использованием алгоритма выравнивания износа (Wear Leveling).
 * Вместо перезаписи одной и той же ячейки, запись производится в следующий по порядку слот.
 * Старый слот помечается как невалидный путем обнуления CRC.
 */
void AppEEPROM::Save(const PersistentData& data) {
  PersistentData copy = data;
  copy.seq = ++current_slot_seq_;
  copy.crc = CalculateCrc(copy);  // Вычисление CRC для обеспечения целостности при следующем чтении

  int current_slot = FindActiveSlot();
  int next_slot = (current_slot + 1) % kSlotsCount;

  // Записываем новые данные в следующий по порядку слот
  EEPROM.put(next_slot * kSlotSize, copy);

  // Аннулируем контрольную сумму в старом слоте, чтобы активным считался только новый
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

/**
 * @brief Постановка записи в очередь (Deferred Write).
 * Предотвращает частые циклы записи в память при быстрой смене настроек пользователем.
 * @param data Данные для записи.
 * @param immediate Флаг немедленной записи (игнорирует 5-секундную задержку).
 */
void AppEEPROM::ScheduleSave(const PersistentData& data, bool immediate) {
  pending_data_ = data;
  needs_save_ = true;
  if (immediate) {
    // Устанавливаем время так, чтобы условие в Update() выполнилось немедленно
    last_change_time_ = millis() - kDeferredSaveDelay;
  } else {
    last_change_time_ = millis();
  }
}

/**
 * @brief Метод фонового обслуживания записи.
 * Проверяет, прошло ли достаточно времени с момента последнего изменения.
 */
void AppEEPROM::Update() {
  if (needs_save_ && (millis() - last_change_time_ >= kDeferredSaveDelay)) {
    Save(pending_data_);
    needs_save_ = false;
  }
}
