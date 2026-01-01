#include "sensors.h"

// Подключение необходимых библиотек
#include <Adafruit_BME280.h>
#include "Adafruit_HTU21DF.h"
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
    htu21d = new Adafruit_HTU21DF();
}

bool HTU21DSensor::begin() {
    if (!htu21d->begin(&Wire)) {
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

// Реализация класса SensorManager
SensorManager::SensorManager(int medianWindowSize, float emaAlpha) :
    tempMedianFilter(medianWindowSize), tempEmaFilter(emaAlpha),
    humidityMedianFilter(medianWindowSize), humidityEmaFilter(emaAlpha),
    ds18b20(2) // Предполагаем, что DS18B20 подключен к пину 2
{
    // Конструктор
}

bool SensorManager::begin() {
    bool bme_ok = bme280.begin();
    bool htu_ok = htu21d.begin();
    bool ds_ok = ds18b20.begin();
    return bme_ok && htu_ok && ds_ok;
}

float SensorManager::getFilteredTemperature() {
    // Получаем температуру с разных датчиков
    float bme_temp = bme280.getTemperature();
    float htu_temp = htu21d.getTemperature();
    float ds_temp = ds18b20.getTemperature();
    
    // Выбираем первое доступное значение или усредняем, если несколько доступны
    float temp_values[3];
    int count = 0;
    
    if (!isnan(bme_temp)) {
        temp_values[count++] = bme_temp;
    }
    if (!isnan(htu_temp)) {
        temp_values[count++] = htu_temp;
    }
    if (!isnan(ds_temp)) {
        temp_values[count++] = ds_temp;
    }
    
    if (count == 0) {
        return NAN; // Нет доступных значений
    }
    
    // Если только одно значение - используем его
    float raw_temp;
    if (count == 1) {
        raw_temp = temp_values[0];
    } else {
        // Иначе усредняем доступные значения
        float sum = 0;
        for (int i = 0; i < count; i++) {
            sum += temp_values[i];
        }
        raw_temp = sum / count;
    }
    
    // Применяем фильтры: сначала медианный, затем EMA
    float median_filtered = tempMedianFilter.apply(raw_temp);
    float final_filtered = tempEmaFilter.apply(median_filtered);
    
    return final_filtered;
}

float SensorManager::getFilteredHumidity() {
    // Получаем влажность с разных датчиков
    float bme_humidity = bme280.getHumidity();
    float htu_humidity = htu21d.getHumidity();
    
    // Выбираем первое доступное значение или усредняем, если несколько доступны
    float humidity_values[2];
    int count = 0;
    
    if (!isnan(bme_humidity)) {
        humidity_values[count++] = bme_humidity;
    }
    if (!isnan(htu_humidity)) {
        humidity_values[count++] = htu_humidity;
    }
    
    if (count == 0) {
        return NAN; // Нет доступных значений
    }
    
    // Если только одно значение - используем его
    float raw_humidity;
    if (count == 1) {
        raw_humidity = humidity_values[0];
    } else {
        // Иначе усредняем доступные значения
        float sum = 0;
        for (int i = 0; i < count; i++) {
            sum += humidity_values[i];
        }
        raw_humidity = sum / count;
    }
    
    // Применяем фильтры: сначала медианный, затем EMA
    float median_filtered = humidityMedianFilter.apply(raw_humidity);
    float final_filtered = humidityEmaFilter.apply(median_filtered);
    
    return final_filtered;
}

void SensorManager::resetFilters() {
    tempMedianFilter.reset();
    tempEmaFilter.reset();
    humidityMedianFilter.reset();
    humidityEmaFilter.reset();
}