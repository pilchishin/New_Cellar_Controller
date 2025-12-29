#include "sensors.h"

// Подключение необходимых библиотек для датчиков
#ifdef ARDUINO_ARCH_AVR
#include <OneWire.h>
#include <DallasTemperature.h>
#endif

// Библиотеки для датчиков
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "SparkFunHTU21D.h"
#include <Wire.h>  // Добавляем библиотеку для I2C

// Глобальные объекты для датчиков
Adafruit_BME280 bme;
HTU21D htu21d;

#ifdef ARDUINO_ARCH_AVR
// Пины для датчиков (настройки для Arduino Nano)
#define ONE_WIRE_BUS 2  // Пин для DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors_ds18b20(&oneWire);
#endif

// Конструктор SensorManager
SensorManager::SensorManager() {
    // Инициализация флагов инициализации
    bme280Initialized = false;
    htu21dInitialized = false;
    ds18b20Initialized = false;
    
    // Установка начальных значений
    bme280Temperature = DEVICE_DISCONNECTED_C;
    bme280Humidity = 0.0f;
    htu21dTemperature = DEVICE_DISCONNECTED_C;
    htu21dHumidity = 0.0f;
    ds18b20Temperature = DEVICE_DISCONNECTED_C;
    
    // Установка флагов валидности
    bme280TempValid = false;
    bme280HumidityValid = false;
    htu21dTempValid = false;
    htu21dHumidityValid = false;
    ds18b20TempValid = false;
    
    // Инициализация таймеров
    lastBmeReadTime = 0;
    lastHtuReadTime = 0;
    lastDsReadTime = 0;
}

void SensorManager::init() {
    // Инициализация I2C
    Wire.begin();
    
    // Инициализация BME280
    bme280Initialized = bme.begin(0x76); // Адрес по умолчанию для BME280
    if (!bme280Initialized) {
        bme280Initialized = bme.begin(0x77); // Альтернативный адрес
    }
    
    // Инициализация HTU21D
    htu21dInitialized = initHTU21D();
    
    // Инициализация DS18B20
    ds18b20Initialized = initDS18B20();
    
    // Если датчики не инициализированы, установить соответствующие флаги
    if (!bme280Initialized) {
        bme280TempValid = false;
        bme280HumidityValid = false;
    }
    
    if (!htu21dInitialized) {
        htu21dTempValid = false;
        htu21dHumidityValid = false;
    }
    
    if (!ds18b20Initialized) {
        ds18b20TempValid = false;
    }
}

void SensorManager::readSensors() {
    // Чтение данных с BME280
    if (bme280Initialized) {
        bme280Temperature = bme.readTemperature();
        bme280Humidity = bme.readHumidity();
        
        // Проверка валидности данных
        bme280TempValid = (bme280Temperature != DEVICE_DISCONNECTED_C && 
                          !isnan(bme280Temperature) && 
                          abs(bme280Temperature) != 85.0); // 85°C - ошибка BME280
        
        bme280HumidityValid = (bme280Humidity != DEVICE_DISCONNECTED_C && 
                              !isnan(bme280Humidity) && 
                              bme280Humidity >= 0.0f && 
                              bme280Humidity <= 100.0f);
    }
    
    // Чтение данных с HTU21D
    if (htu21dInitialized) {
        float temp = htu21d.readTemperature();
        float humidity = htu21d.readHumidity();
        
        htu21dTemperature = temp;
        htu21dHumidity = humidity;
        
        // Проверка валидности данных
        htu21dTempValid = (htu21dTemperature != DEVICE_DISCONNECTED_C && 
                          !isnan(htu21dTemperature));
        
        htu21dHumidityValid = (htu21dHumidity != DEVICE_DISCONNECTED_C && 
                              !isnan(htu21dHumidity) && 
                              htu21dHumidity >= 0.0f && 
                              htu21dHumidity <= 100.0f);
    }
    
    // Чтение данных с DS18B20
    if (ds18b20Initialized) {
        float temp;
        if (readDS18B20Temp(temp)) {
            ds18b20Temperature = temp;
            ds18b20TempValid = true;
        } else {
            ds18b20TempValid = false;
        }
    }
}

void SensorManager::updateSensors() {
    unsigned long currentTime = millis();
    
    // Обновление данных с BME280
    if (bme280Initialized && (currentTime - lastBmeReadTime >= bmeReadInterval)) {
        bme280Temperature = bme.readTemperature();
        bme280Humidity = bme.readHumidity();
        
        // Проверка валидности данных
        bme280TempValid = (bme280Temperature != DEVICE_DISCONNECTED_C && 
                          !isnan(bme280Temperature) && 
                          abs(bme280Temperature) != 85.0); // 85°C - ошибка BME280
        
        bme280HumidityValid = (bme280Humidity != DEVICE_DISCONNECTED_C && 
                              !isnan(bme280Humidity) && 
                              bme280Humidity >= 0.0f && 
                              bme280Humidity <= 100.0f);
        
        lastBmeReadTime = currentTime;
    }
    
    // Обновление данных с HTU21D
    if (htu21dInitialized && (currentTime - lastHtuReadTime >= htuReadInterval)) {
        float temp = htu21d.readTemperature();
        float humidity = htu21d.readHumidity();
        
        htu21dTemperature = temp;
        htu21dHumidity = humidity;
        
        // Проверка валидности данных
        htu21dTempValid = (htu21dTemperature != DEVICE_DISCONNECTED_C && 
                          !isnan(htu21dTemperature));
        
        htu21dHumidityValid = (htu21dHumidity != DEVICE_DISCONNECTED_C && 
                              !isnan(htu21dHumidity) && 
                              htu21dHumidity >= 0.0f && 
                              htu21dHumidity <= 100.0f);
        
        lastHtuReadTime = currentTime;
    }
    
    // Обновление данных с DS18B20
    if (ds18b20Initialized && (currentTime - lastDsReadTime >= dsReadInterval)) {
        float temp;
        if (readDS18B20Temp(temp)) {
            ds18b20Temperature = temp;
            ds18b20TempValid = true;
        } else {
            ds18b20TempValid = false;
        }
        
        lastDsReadTime = currentTime;
    }
}

// Методы для получения температуры
float SensorManager::getBME280Temperature() {
    return bme280Temperature;
}

float SensorManager::getHTU21DTemperature() {
    return htu21dTemperature;
}

float SensorManager::getDS18B20Temperature() {
    return ds18b20Temperature;
}

// Методы для получения влажности
float SensorManager::getBME280Humidity() {
    return bme280Humidity;
}

float SensorManager::getHTU21DHumidity() {
    return htu21dHumidity;
}

// Методы для получения температуры (новые объявления)
float SensorManager::getBmeTemp() {
    return bme280Temperature;
}

float SensorManager::getHtuTemp() {
    return htu21dTemperature;
}

float SensorManager::getDsTemp() {
    return ds18b20Temperature;
}

// Методы для получения влажности (новые объявления)
float SensorManager::getBmeHum() {
    return bme280Humidity;
}

float SensorManager::getHtuHum() {
    return htu21dHumidity;
}

// Методы проверки валидности
bool SensorManager::isBME280TemperatureValid() {
    return bme280TempValid;
}

bool SensorManager::isBME280HumidityValid() {
    return bme280HumidityValid;
}

bool SensorManager::isHTU21DTemperatureValid() {
    return htu21dTempValid;
}

bool SensorManager::isHTU21DHumidityValid() {
    return htu21dHumidityValid;
}

bool SensorManager::isDS18B20TemperatureValid() {
    return ds18b20TempValid;
}

// Методы для работы с DS18B20
bool SensorManager::initDS18B20() {
#ifdef ARDUINO_ARCH_AVR
    sensors_ds18b20.begin();
    
    // Проверка наличия датчиков
    if (sensors_ds18b20.getDeviceCount() == 0) {
        return false;
    }
    
    // Установка разрешения измерения
    setDS18B20Resolution();
    
    // Валидация датчика
    return validateDS18B20();
#else
    return false;
#endif
}

bool SensorManager::readDS18B20Temp(float& temp) {
#ifdef ARDUINO_ARCH_AVR
    // Запуск преобразования температуры
    sensors_ds18b20.requestTemperatures();
    
    // Чтение температуры
    temp = sensors_ds18b20.getTempCByIndex(0);
    
    // Проверка валидности результата
    if (temp == DEVICE_DISCONNECTED_C || isnan(temp)) {
        return false;
    }
    
    return true;
#else
    temp = DEVICE_DISCONNECTED_C;
    return false;
#endif
}

void SensorManager::setDS18B20Resolution() {
#ifdef ARDUINO_ARCH_AVR
    // Установка максимального разрешения 12 бит (0.0625°C)
    sensors_ds18b20.setResolution(12);
#endif
}

bool SensorManager::validateDS18B20() {
#ifdef ARDUINO_ARCH_AVR
    float temp;
    return readDS18B20Temp(temp);
#else
    return false;
#endif
}

// Методы для работы с HTU21D
bool SensorManager::initHTU21D() {
    // Инициализация датчика
    if (!htu21d.begin()) {
        return false;
    }
    
    // Сброс датчика
    if (!resetHTU21D()) {
        return false;
    }
    
    // Установка разрешения измерений
    htu21d.setResolution(HTU21D_RES_RH12_TEMP14); // 12 бит для влажности, 14 для температуры
    
    // Отключение режима удержания (hold mode)
    // В библиотеке SparkFun HTU21D это настраивается автоматически
    
    // Валидация датчика
    return validateHTU21D();
}

bool SensorManager::resetHTU21D() {
    // Отправка команды сброса
    Wire.beginTransmission(0x40); // Адрес HTU21D
    Wire.write(0xFE); // Команда сброса
    uint8_t result = Wire.endTransmission(true);
    
    // Задержка после сброса (не более 15 мс)
    delay(15);
    
    return (result == 0);
}

bool SensorManager::readHTU21DTemperature(float& temp) {
    temp = htu21d.readTemperature();
    
    if (isnan(temp)) {
        return false;
    }
    
    return true;
}

bool SensorManager::readHTU21DHumidity(float& humidity) {
    humidity = htu21d.readHumidity();
    
    if (isnan(humidity)) {
        return false;
    }
    
    return true;
}

bool SensorManager::validateHTU21D() {
    float temp, humidity;
    
    if (!readHTU21DTemperature(temp)) {
        return false;
    }
    
    if (!readHTU21DHumidity(humidity)) {
        return false;
    }
    
    // Проверка разумности значений
    if (temp < -40.0f || temp > 125.0f) {
        return false;
    }
    
    if (humidity < 0.0f || humidity > 100.0f) {
        return false;
    }
    
    return true;
}

bool SensorManager::readHTU21DRegister(uint8_t reg, uint8_t& value) {
    Wire.beginTransmission(0x40); // Адрес HTU21D
    Wire.write(reg);
    uint8_t result = Wire.endTransmission(false);
    
    if (result != 0) {
        return false;
    }
    
    Wire.requestFrom(0x40, 1);
    if (Wire.available() < 1) {
        return false;
    }
    
    value = Wire.read();
    return true;
}

bool SensorManager::writeHTU21DRegister(uint8_t reg, uint8_t value) {
    Wire.beginTransmission(0x40); // Адрес HTU21D
    Wire.write(reg);
    Wire.write(value);
    uint8_t result = Wire.endTransmission(true);
    
    return (result == 0);
}

bool SensorManager::checkCRC(uint16_t data, uint8_t crc) {
    // CRC-8 для HTU21D
    uint8_t crcValue = 0x00;
    uint8_t currentByte = (data >> 8) & 0xFF;
    
    // Обработка старшего байта
    for (int i = 0; i < 8; i++) {
        if ((crcValue ^ currentByte) & 0x80) {
            crcValue = (crcValue << 1) ^ 0x31;
        } else {
            crcValue <<= 1;
        }
        currentByte <<= 1;
    }
    
    // Обработка младшего байта
    currentByte = data & 0xFF;
    for (int i = 0; i < 8; i++) {
        if ((crcValue ^ currentByte) & 0x80) {
            crcValue = (crcValue << 1) ^ 0x31;
        } else {
            crcValue <<= 1;
        }
        currentByte <<= 1;
    }
    
    return (crcValue == crc);
}
