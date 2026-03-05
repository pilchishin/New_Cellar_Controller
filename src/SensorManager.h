#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_HTU21DF.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "Types.h"
#include "Config.h" // Предполагается, что здесь задан пин ONE_WIRE_BUS
#include "MathUtils.h"

class SensorManager {
private:
    // Объекты библиотек для работы с датчиками
    Adafruit_BME280 bme;
    Adafruit_HTU21DF htu;
    OneWire oneWire;
    DallasTemperature dsSensor;

    // Структуры для хранения финальных (отфильтрованных) данных
    SensorData insideData;  // Данные внутри (BME280)
    SensorData outsideData; // Данные снаружи (HTU21D)
    float controlTemp;      // Контрольная температура подвала (DS18B20)

    // Объекты фильтров (Медиана N=3 + EMA alpha=0.2)
    // Создаем отдельный фильтр для каждого физического параметра
    Filter filterBmeTemp;
    Filter filterBmeHum;
    Filter filterHtuTemp;
    Filter filterHtuHum;
    Filter filterDsTemp;

    // Флаги состояния датчиков (исправен/неисправен)
    bool bmeValid;
    bool htuValid;
    bool dsValid;

    // Параметры повторных попыток
    uint8_t bmeRetries;
    uint8_t htuRetries;
    uint8_t dsRetries;
    unsigned long lastBmeRetry;
    unsigned long lastHtuRetry;
    unsigned long lastDsRetry;
    const uint8_t MAX_RETRIES = 3;
    const uint32_t RETRY_INTERVAL = 60000UL;

    // Методы инициализации конкретных датчиков
    void initBme();
    void initHtu();
    void initDs();

    // Счетчики ошибок для I2C устройств
    uint8_t i2cErrorCount;
    const uint8_t I2C_MAX_ERRORS = 3;

    // Калибровочные коэффициенты
    CalibrationData calib;

public:
    // Конструктор: инициализируем OneWire пином из Config.h
    SensorManager();

    // Инициализация шин и самих датчиков
    void init();

    // Главный метод опроса, вызывается каждые 10 секунд из loop()
    void update();

    // Возвращает код ошибки, если что-то пошло не так
    ErrorCode checkErrors();

    // Попытка восстановления после сбоя I2C
    void recover();

    // Геттеры для получения актуальных данных контроллером
    SensorData getInside() const { return insideData; }
    SensorData getOutside() const { return outsideData; }
    float getControlTemp() const { return controlTemp; }
    bool isI2CFailing() const { return i2cErrorCount >= I2C_MAX_ERRORS; }

    void setCalibration(const CalibrationData& data) { calib = data; }
};

#endif