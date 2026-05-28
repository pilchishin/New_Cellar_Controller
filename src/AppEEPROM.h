#ifndef APP_EEPROM_H
#define APP_EEPROM_H

#include <Arduino.h>
#include <stddef.h>
#include "Types.h"

/**
 * @struct PersistentData
 * @brief Структура всех данных, сохраняемых в энергонезависимой памяти (EEPROM).
 * Содержит настройки целевого климата, калибровочные коэффициенты, статистику работы системы
 * и контрольную сумму CRC для проверки целостности.
 */
struct PersistentData {
    float targetTemp;            ///< Целевая температура внутри подвала (°C)
    float targetRh;              ///< Целевая относительная влажность (%)
    CalibrationData calibration; ///< Данные калибровки датчиков (смещения)
    SystemStatistics stats;      ///< Накопленная статистика работы (время наработки)
    uint16_t seq;                ///< Порядковый номер записи (sequence counter)
    uint16_t crc;                ///< Контрольная сумма CRC16 (MODBUS-совместимая)
};

static_assert(offsetof(PersistentData, crc) ==
sizeof(PersistentData) - sizeof(uint16_t),
"crc must be the last field in PersistentData");

/**
 * @class AppEEPROM
 * @brief Класс для управления постоянной памятью контроллера.
 * Реализует механизм выравнивания износа (Wear Leveling) путем циклической записи в 10 слотов,
 * а также механизм отложенной записи (Deferred Save) для предотвращения слишком частого
 * обращения к физической ячейке памяти при изменении параметров.
 */
class AppEEPROM {
 private:
  static const uint16_t kEepromSize = 1024;   ///< Доступный объем EEPROM для ATmega328P (1 КБ)
  static const uint8_t kSlotsCount = 10;      ///< Количество слотов для циклической записи (wear leveling)
  static const uint16_t kSlotSize = sizeof(PersistentData); ///< Размер одного слота в байтах
  static const uint32_t kDeferredSaveDelay = 5000UL;        ///< Задержка отложенного сохранения (5 секунд)

  // Проверка на этапе компиляции, что слоты помещаются в доступный объем памяти
  static_assert(kSlotsCount * kSlotSize <= kEepromSize,
                "EEPROM: slots overflow kEepromSize");

  /**
   * @brief Вычисляет контрольную сумму CRC16 для структуры данных.
   * @param data Ссылка на структуру для расчета.
   * @return 16-битное значение контрольной суммы.
   */
  uint16_t CalculateCrc(const PersistentData& data);

  /**
   * @brief Ищет индекс активного (последнего валидного) слота в EEPROM.
   * @return Индекс слота (0-9) или -1, если валидных данных не найдено.
   */
  int FindActiveSlot();

  uint16_t current_slot_seq_ = 0; ///< Текущий порядковый номер записи
  PersistentData pending_data_; ///< Буфер данных, ожидающих записи
  bool needs_save_;             ///< Флаг наличия изменений, требующих сохранения
  unsigned long last_change_time_; ///< Время последнего изменения данных (в мс)

 public:
  /**
   * @brief Конструктор по умолчанию.
   */
  AppEEPROM();

  /**
   * @brief Загружает данные из последнего активного слота EEPROM.
   * Если валидные данные отсутствуют, инициализирует структуру значениями по умолчанию.
   * @param data Ссылка на структуру, куда будут помещены данные.
   */
  void Load(PersistentData& data);

  /**
   * @brief Немедленно сохраняет данные в следующий доступный слот EEPROM.
   * Реализует логику wear leveling и аннулирует CRC в старом слоте.
   * @param data Структура данных для сохранения.
   */
  void Save(const PersistentData& data);

  /**
   * @brief Планирует сохранение данных.
   * @param data Данные для записи.
   * @param immediate Если true, запись произойдет при следующем вызове Update() без ожидания 5с.
   */
  void ScheduleSave(const PersistentData& data, bool immediate = false);

  /**
   * @brief Обслуживает таймер отложенной записи.
   * Должен вызываться в главном цикле (loop) контроллера.
   */
  void Update();
};

#endif
