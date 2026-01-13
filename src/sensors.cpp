#include "sensors.h"
#include "config.h"

#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_HTU21DF.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/* ============================================================
 * Глобальные объекты библиотек
 * (инкапсулированы внутри .cpp)
 * ============================================================ */

static Adafruit_BME280     g_bme280;
static Adafruit_HTU21DF   g_htu21d;
static OneWire*           g_oneWire = nullptr;
static DallasTemperature* g_ds18b20 = nullptr;

/* ============================================================
 * SensorBase
 * ============================================================ */

SensorBase::SensorBase(SensorType type, bool hasHumidity)
    : _type(type),
      _state(SensorState::ERROR),
      _tempOffset(0.0f),
      _humOffset(0.0f),
      _hasHumidity(hasHumidity)
{
    _raw = { NAN, NAN, hasHumidity };
    _filtered = { NAN, NAN, hasHumidity };
}

void SensorBase::resetError()
{
    _state = SensorState::OK; //  Сброс ошибки (ручной) */
}

SensorState SensorBase::getState() const
{
    return _state;
}

SensorType SensorBase::getType() const
{
    return _type;
}

const SensorRawData& SensorBase::getRawData() const
{
    return _raw;
}

const SensorFilteredData& SensorBase::getFilteredData() const
{
    return _filtered;
}

void SensorBase::setError()
{
    _state = SensorState::ERROR;
}

void SensorBase::applyOffsets(SensorRawData& data)
{
    data.temperature += _tempOffset;

    if (data.hasHumidity) {
        data.humidity += _humOffset;
    }
}

bool SensorBase::validate(const SensorRawData& data)
{
    if (isnan(data.temperature)) {
        return false;
    }

    if (data.temperature < -40.0f || data.temperature > 85.0f) {
        return false;
    }

    if (data.hasHumidity) {
        if (isnan(data.humidity)) {
            return false;
        }

        if (data.humidity < 0.0f || data.humidity > 100.0f) {
            return false;
        }
    }

    return true;
}

/* ============================================================
 * SensorBME280
 * ============================================================ */

SensorBME280::SensorBME280()
    : SensorBase(SensorType::BME280_IN, true)
{
}

bool SensorBME280::begin()
{
    if (!g_bme280.begin(0x76)) {
        setError();
        return false;
    }

    _state = SensorState::OK;
    return true;
}

bool SensorBME280::readOnce()
{
    SensorRawData data;
    data.hasHumidity = true;

    data.temperature = g_bme280.readTemperature();
    data.humidity    = g_bme280.readHumidity();

    applyOffsets(data);

    if (!validate(data)) {
        return false;
    }

    _raw = data;
    _filtered.temperature = data.temperature;
    _filtered.humidity    = data.humidity;

    return true;
}

bool SensorBME280::read()
{
    for (uint8_t i = 0; i < SENSOR_READ_RETRIES; i++) {
        if (readOnce()) {
            _state = SensorState::OK;
            return true;
        }
    }

    setError();
    return false;
}

/* ============================================================
 * SensorHTU21D
 * ============================================================ */

SensorHTU21D::SensorHTU21D()
    : SensorBase(SensorType::HTU21D_OUT, true)
{
}

bool SensorHTU21D::begin()
{
    if (!g_htu21d.begin()) {
        setError();
        return false;
    }

    _state = SensorState::OK;
    return true;
}

bool SensorHTU21D::readOnce()
{
    SensorRawData data;
    data.hasHumidity = true;

    data.temperature = g_htu21d.readTemperature();
    data.humidity    = g_htu21d.readHumidity();

    applyOffsets(data);

    if (!validate(data)) {
        return false;
    }

    _raw = data;
    _filtered.temperature = data.temperature;
    _filtered.humidity    = data.humidity;

    return true;
}

bool SensorHTU21D::read()
{
    for (uint8_t i = 0; i < SENSOR_READ_RETRIES; i++) {
        if (readOnce()) {
            _state = SensorState::OK;
            return true;
        }
    }

    setError();
    return false;
}

/* ============================================================
 * SensorDS18B20
 * ============================================================ */

SensorDS18B20::SensorDS18B20(uint8_t oneWirePin)
    : SensorBase(SensorType::DS18B20_CTRL, false),
      _pin(oneWirePin)
{
}

bool SensorDS18B20::begin()
{
    g_oneWire = new OneWire(_pin);
    g_ds18b20 = new DallasTemperature(g_oneWire);

    g_ds18b20->begin();

    if (g_ds18b20->getDeviceCount() == 0) {
        setError();
        return false;
    }

    _state = SensorState::OK;
    return true;
}

bool SensorDS18B20::readOnce()
{
    g_ds18b20->requestTemperatures();
    float temp = g_ds18b20->getTempCByIndex(0);

    SensorRawData data;
    data.hasHumidity = false;
    data.temperature = temp;
    data.humidity = NAN;

    applyOffsets(data);

    if (!validate(data)) {
        return false;
    }

    _raw = data;
    _filtered.temperature = data.temperature;

    return true;
}

bool SensorDS18B20::read()
{
    for (uint8_t i = 0; i < SENSOR_READ_RETRIES; i++) {
        if (readOnce()) {
            _state = SensorState::OK;
            return true;
        }
    }

    setError();
    return false;
}

/* ============================================================
 * SensorsManager
 * ============================================================ */

SensorsManager::SensorsManager()
    : _lastPollMs(0),
      _bme280(),
      _htu21d(),
      _ds18b20(DS18B20_PIN)
{
}

bool SensorsManager::begin()
{
    bool ok = true;

    ok &= _bme280.begin();
    ok &= _htu21d.begin();
    ok &= _ds18b20.begin();

    _lastPollMs = millis();

    return ok;
}

void SensorsManager::update()
{
    unsigned long now = millis();

    if (now - _lastPollMs < SENSOR_POLL_INTERVAL_MS) {
        return;
    }

    _lastPollMs = now;

    _bme280.read();
    _htu21d.read();
    _ds18b20.read();
}

bool SensorsManager::hasError() const
{
    return (_bme280.getState() == SensorState::ERROR ||
            _htu21d.getState() == SensorState::ERROR ||
            _ds18b20.getState() == SensorState::ERROR);
}

SensorBME280& SensorsManager::bme()
{
    return _bme280;
}

SensorHTU21D& SensorsManager::htu()
{
    return _htu21d;
}

SensorDS18B20& SensorsManager::ds18()
{
    return _ds18b20;
}
