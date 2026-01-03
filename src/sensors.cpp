#include "sensors.h"
#include "config.h"

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
DS18B20Sensor::DS18B20Sensor(int DS18B20_PIN) {
    this->DS18B20_PIN = DS18B20_PIN;
    oneWire = OneWire(DS18B20_PIN);
    sensors = DallasTemperature(oneWire);
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
    ds18b20(2), // Предполагаем, что DS18B20 подключен к пину 2
    lastPollTime(0),
    initialized(false),
    lastBmeTemp(NAN),
    lastBmeHumidity(NAN),
    lastHtuTemp(NAN),
    lastHtuHumidity(NAN),
    lastDsTemp(NAN),
    bmeAvailable(false),
    htuAvailable(false),
    dsAvailable(false)
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
    
      // Применяем фильтры: сначала медианный, затем EMA
    float median_filtered = tempMedianFilter.apply(raw_temp);
    float final_filtered = tempEmaFilter.apply(median_filtered);
    
    return final_filtered;
}

float SensorManager::getFilteredHumidity() {
    // Получаем влажность с разных датчиков
    float bme_humidity = bme280.getHumidity();
    float htu_humidity = htu21d.getHumidity();
    
      
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

bool SensorManager::update() {
    unsigned long currentTime = millis();
    
    // Проверяем, прошло ли достаточно времени с последнего опроса
    if (!initialized || (currentTime - lastPollTime >= SENSOR_POLL_INTERVAL_MS)) {
        // Обновляем время последнего опроса
        lastPollTime = currentTime;
        initialized = true;
        
        // Выполняем опрос датчиков и применяем фильтры
        float bme_temp = bme280.getTemperature();
        float htu_temp = htu21d.getTemperature();
        float ds_temp = ds18b20.getTemperature();
        float bme_humidity = bme280.getHumidity();
        float htu_humidity = htu21d.getHumidity();
        
        // Обновляем флаги доступности датчиков
        bmeAvailable = !isnan(bme_temp) && !isnan(bme_humidity);
        htuAvailable = !isnan(htu_temp) && !isnan(htu_humidity);
        dsAvailable = !isnan(ds_temp);
        
        // Сохраняем последние значения
        if (!isnan(bme_temp)) lastBmeTemp = bme_temp;
        if (!isnan(bme_humidity)) lastBmeHumidity = bme_humidity;
        if (!isnan(htu_temp)) lastHtuTemp = htu_temp;
        if (!isnan(htu_humidity)) lastHtuHumidity = htu_humidity;
        if (!isnan(ds_temp)) lastDsTemp = ds_temp;
        
        // Обработка температуры
        float temp_values[3];
        int temp_count = 0;
        
        if (!isnan(bme_temp)) {
            temp_values[temp_count++] = bme_temp;
        }
        if (!isnan(htu_temp)) {
            temp_values[temp_count++] = htu_temp;
        }
        if (!isnan(ds_temp)) {
            temp_values[temp_count++] = ds_temp;
        }
        
        float filtered_temp = NAN;
        if (temp_count > 0) {
            float raw_temp;
            if (temp_count == 1) {
                raw_temp = temp_values[0];
            } else {
                // Усредняем доступные значения
                float sum = 0;
                for (int i = 0; i < temp_count; i++) {
                    sum += temp_values[i];
                }
                raw_temp = sum / temp_count;
            }
            
            // Применяем фильтры: сначала медианный, затем EMA
            float median_filtered = tempMedianFilter.apply(raw_temp);
            filtered_temp = tempEmaFilter.apply(median_filtered);
        }
        
        // Обработка влажности
        float humidity_values[2];
        int humidity_count = 0;
        
        if (!isnan(bme_humidity)) {
            humidity_values[humidity_count++] = bme_humidity;
        }
        if (!isnan(htu_humidity)) {
            humidity_values[humidity_count++] = htu_humidity;
        }
        
        float filtered_humidity = NAN;
        if (humidity_count > 0) {
            float raw_humidity;
            if (humidity_count == 1) {
                raw_humidity = humidity_values[0];
            } else {
                // Усредняем доступные значения
                float sum = 0;
                for (int i = 0; i < humidity_count; i++) {
                    sum += humidity_values[i];
                }
                raw_humidity = sum / humidity_count;
            }
            
            // Применяем фильтры: сначала медианный, затем EMA
            float median_filtered = humidityMedianFilter.apply(raw_humidity);
            filtered_humidity = humidityEmaFilter.apply(median_filtered);
        }
        
        // Вывод результатов в лог
        Serial.print("Sensor Poll - ");
        Serial.print("Time: "); Serial.print(currentTime);
        Serial.print("ms, ");
        
        if (bmeAvailable) {
            Serial.print("BME280: T="); Serial.print(bme_temp, 2); Serial.print("C, H="); Serial.print(bme_humidity, 1); Serial.print("%, ");
        } else {
            Serial.print("BME280: N/A, ");
        }
        
        if (htuAvailable) {
            Serial.print("HTU21DF: T="); Serial.print(htu_temp, 2); Serial.print("C, H="); Serial.print(htu_humidity, 1); Serial.print("%, ");
        } else {
            Serial.print("HTU21DF: N/A, ");
        }
        
        if (dsAvailable) {
            Serial.print("DS18B20: T="); Serial.print(ds_temp, 2); Serial.print("C, ");
        } else {
            Serial.print("DS18B20: N/A, ");
        }
        
        if (!isnan(filtered_temp)) {
            Serial.print("Filtered T="); Serial.print(filtered_temp, 2); Serial.print("C, ");
        }
        
        if (!isnan(filtered_humidity)) {
            Serial.print("Filtered H="); Serial.print(filtered_humidity, 1); Serial.print("%");
        }
        
        Serial.println();
        
        return true;
    }
    
    return false;
}