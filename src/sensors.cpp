#include "sensors.h"

// Подключение необходимых библиотек
#include <Adafruit_BME280.h>
#include "SparkFunHTU21D.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// Реализация класса BME280Sensor
BME280Sensor::BME280Sensor() {
    bme = new Adafruit_BME280();
}

bool BME280Sensor::begin(uint8_t addr) {
    if (!bme->begin(addr)) {
        return false;
    }
    return true;
}

float BME280Sensor::getTemperature() {
    float temp = bme->readTemperature();
    if (isnan(temp)) {
        return NAN;
    }
    return temp;
}

float BME280Sensor::getHumidity() {
    float humidity = bme->readHumidity();
    if (isnan(humidity)) {
        return NAN;
    }
    return humidity;
}

// Реализация класса HTU21DSensor
HTU21DSensor::HTU21DSensor() {
    htu21d = new HTU21D();
}

bool HTU21DSensor::begin() {
    if (!htu21d->begin()) {
        return false;
    }
    return true;
}

float HTU21DSensor::getTemperature() {
    float temp = htu21d->readTemperature();
    if (isnan(temp)) {
        return NAN;
    }
    return temp;
}

float HTU21DSensor::getHumidity() {
    float humidity = htu21d->readHumidity();
    if (isnan(humidity)) {
        return NAN;
    }
    return humidity;
}

// Реализация класса DS18B20Sensor
DS18B20Sensor::DS18B20Sensor(int pin) {
    this->pin = pin;
    oneWire = new OneWire(pin);
    sensors = new DallasTemperature(oneWire);
}

bool DS18B20Sensor::begin() {
    sensors->begin();
    if (sensors->getDeviceCount() == 0) {
        return false;
    }
    return true;
}

float DS18B20Sensor::getTemperature() {
    sensors->requestTemperatures();
    float temp = sensors->getTempCByIndex(0);
    if (temp == DEVICE_DISCONNECTED_C) {
        return NAN;
    }
    return temp;
}