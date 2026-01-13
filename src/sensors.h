#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

/* ============================================================
 * Общие типы и перечисления
 * ============================================================ */

/**
 * Общее состояние датчика.
 * Любая ошибка переводит датчик в ERROR и требует ручного сброса.
 */
enum class SensorState : uint8_t {
    OK,
    ERROR
};

/**
 * Тип датчика — нужен для логирования, меню и диагностики
 */
enum class SensorType : uint8_t {
    BME280_IN,
    HTU21D_OUT,
    DS18B20_CTRL
};

/**
 * Структура "сырых" данных датчика
 * (до фильтрации)
 */
struct SensorRawData {
    float temperature;   // °C
    float humidity;      // %RH (если есть)
    bool  hasHumidity;   // true для BME280/HTU21D
};

/**
 * Структура данных после фильтрации
 */
struct SensorFilteredData {
    float temperature;   // °C
    float humidity;      // %RH (если есть)
    bool  hasHumidity;
};

/* ============================================================
 * Базовый абстрактный класс датчика
 * ============================================================ */

/**
 * Абстрактный базовый класс.
 * Определяет единый интерфейс для всех датчиков.
 */
class SensorBase {
public:
    virtual ~SensorBase() {}

    /** Инициализация датчика */
    virtual bool begin() = 0;

    /** Чтение данных с повторными попытками */
    virtual bool read() = 0;

    /** Сброс ошибки (ручной) */
    virtual void resetError();

    /** Проверка состояния */
    SensorState getState() const;

    /** Тип датчика */
    SensorType getType() const;

    /** Сырые данные */
    const SensorRawData& getRawData() const;

    /** Отфильтрованные данные */
    const SensorFilteredData& getFilteredData() const;

protected:
    SensorBase(SensorType type, bool hasHumidity);

    /** Применение оффсетов */
    void applyOffsets(SensorRawData& data);

    /** Валидация данных */
    bool validate(const SensorRawData& data);

    /** Перевод в состояние ошибки */
    void setError();

protected:
    SensorType          _type;
    SensorState         _state;

    SensorRawData       _raw;
    SensorFilteredData  _filtered;

    float _tempOffset;   // °C
    float _humOffset;    // %RH

    bool  _hasHumidity;
};

/* ============================================================
 * BME280 (внутренний датчик)
 * ============================================================ */

class SensorBME280 : public SensorBase {
public:
    SensorBME280();

    bool begin() override;
    bool read() override;

private:
    bool readOnce();
};

/* ============================================================
 * HTU21D (уличный датчик)
 * ============================================================ */

class SensorHTU21D : public SensorBase {
public:
    SensorHTU21D();

    bool begin() override;
    bool read() override;

private:
    bool readOnce();
};

/* ============================================================
 * DS18B20 (контрольный датчик)
 * ============================================================ */

class SensorDS18B20 : public SensorBase {
public:
    SensorDS18B20(uint8_t oneWirePin);

    bool begin() override;
    bool read() override;

private:
    uint8_t _pin;
    bool readOnce();
};

/* ============================================================
 * Менеджер датчиков
 * ============================================================ */

/**
 * Центральная точка работы с датчиками.
 * Вызывается из loop().
 */
class SensorsManager {
public:
    SensorsManager();

    /** Инициализация всех датчиков */
    bool begin();

    /** Опрос всех датчиков (раз в 10 секунд) */
    void update();

    /** Признак глобальной ошибки */
    bool hasError() const;

    /** Доступ к датчикам */
    SensorBME280&   bme();
    SensorHTU21D&   htu();
    SensorDS18B20&  ds18();

private:
    unsigned long _lastPollMs;

    SensorBME280  _bme280;
    SensorHTU21D  _htu21d;
    SensorDS18B20 _ds18b20;
};

#endif // SENSORS_H

/**
 *                  ┌─────────────────────────┐
                    │      SensorsManager     │
                    │─────────────────────────│
                    │ - lastPollMs            │
                    │─────────────────────────│
                    │ + begin()               │
                    │ + update()              │
                    │ + hasError()            │
                    │ + bme()                 │
                    │ + htu()                 │
                    │ + ds18()                │
                    └─────────┬───────┬───────┘
                              │       │
        ┌─────────────────────┘       └─────────────────────┐
        │                                                   │
┌───────────────┐    ┌───────────────┐      ┌──────────────────┐
│  SensorBME280 │    │ SensorHTU21D  │      │  SensorDS18B20   │
│───────────────│    │───────────────│      │──────────────────│
│ + begin()     │    │ + begin()     │      │ + begin()        │
│ + read()      │    │ + read()      │      │ + read()         │
│ - readOnce()  │    │ - readOnce()  │      │ - readOnce()     │
└───────▲───────┘    └───────▲───────┘      └───────▲──────────┘
        │                    │                          │
        └───────────────┬────┴───────────────┬──────────┘
                        │
              ┌────────────────────┐
              │    SensorBase      │
              │────────────────────│
              │ SensorType         │
              │ SensorState        │
              │ RawData            │
              │ FilteredData       │
              │ Offsets            │
              │────────────────────│
              │ + begin() = 0      │
              │ + read() = 0       │
              │ + resetError()     │
              │ + validate()       │
              │ + applyOffsets()   │
              └────────────────────┘

 */