#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Wire.h>
#include "filters.h"

// Предварительные объявления для библиотек датчиков
class Adafruit_BME280;
class Adafruit_HTU21DF;
class OneWire;
class DallasTemperature;

/**
 * @brief Класс для работы с датчиком BME280
 *
 * Датчик BME280 позволяет измерять температуру и влажность.
 * Использует библиотеку Adafruit BME280.
 */
class BME280Sensor {
public:
    /**
     * @brief Конструктор класса BME280Sensor
     */
    BME280Sensor();
    
    /**
     * @brief Инициализация датчика BME280
     * @param addr Адрес датчика на шине I2C (по умолчанию 0x76)
     * @return true в случае успешной инициализации, false в противном случае
     */
    bool begin(uint8_t addr = 0x76);
    
    /**
     * @brief Получение температуры с датчика BME280
     * @return Температура в градусах Цельсия, или NAN в случае ошибки
     */
    float getTemperature();
    
    /**
     * @brief Получение влажности с датчика BME280
     * @return Влажность в процентах, или NAN в случае ошибки
     */
    float getHumidity();

private:
    Adafruit_BME280* bme;
};

/**
 * @brief Класс для работы с датчиком HTU21D
 *
 * Датчик HTU21D позволяет измерять температуру и влажность.
 * Использует библиотеку SparkFun HTU21D.
 */
class HTU21DSensor {
public:
    /**
     * @brief Конструктор класса HTU21DSensor
     */
    HTU21DSensor();
    
    /**
     * @brief Инициализация датчика HTU21D
     * @return true в случае успешной инициализации, false в противном случае
     */
    bool begin();
    
    /**
     * @brief Получение температуры с датчика HTU21D
     * @return Температура в градусах Цельсия, или NAN в случае ошибки
     */
    float getTemperature();
    
    /**
     * @brief Получение влажности с датчика HTU21D
     * @return Влажность в процентах, или NAN в случае ошибки
     */
    float getHumidity();

private:
    Adafruit_HTU21DF* htu21d;
};

/**
 * @brief Класс для работы с датчиком DS18B20
 *
 * Датчик DS18B20 позволяет измерять температуру.
 * Использует библиотеки OneWire и DallasTemperature.
 */
class DS18B20Sensor {
public:
    /**
     * @brief Конструктор класса DS18B20Sensor
     * @param pin Пин, к которому подключен датчик
     */
    DS18B20Sensor(int pin);
    
    /**
     * @brief Инициализация датчика DS18B20
     * @return true в случае успешной инициализации, false в противном случае
     */
    bool begin();
    
    /**
     * @brief Получение температуры с датчика DS18B20
     * @return Температура в градусах Цельсия, или NAN в случае ошибки
     */
    float getTemperature();

private:
    int pin;
    OneWire* oneWire;
    DallasTemperature* sensors;
};

/**
 * @brief Класс для управления сенсорами с фильтрацией данных
 *
 * SensorManager объединяет работу с различными датчиками и применяет
 * фильтры к полученным данным для уменьшения шума и выбросов.
 */
class SensorManager {
public:
    /**
     * @brief Конструктор класса SensorManager
     * @param medianWindowSize Размер окна для медианного фильтра (по умолчанию 3)
     * @param emaAlpha Коэффициент альфа для EMA фильтра (по умолчанию 0.2f)
     */
    SensorManager(int medianWindowSize = 3, float emaAlpha = 0.2f);
    
    /**
     * @brief Инициализация всех датчиков
     * @return true в случае успешной инициализации всех датчиков, false в противном случае
     */
    bool begin();
    
    /**
     * @brief Получение отфильтрованной температуры с датчиков
     * @return Температура в градусах Цельсия, или NAN в случае ошибки
     */
    float getFilteredTemperature();
    
    /**
     * @brief Получение отфильтрованной влажности с датчиков
     * @return Влажность в процентах, или NAN в случае ошибки
     */
    float getFilteredHumidity();
    
    /**
     * @brief Сброс всех фильтров
     */
    void resetFilters();

private:
    BME280Sensor bme280;
    HTU21DSensor htu21d;
    DS18B20Sensor ds18b20;
    
    MedianFilter<float> tempMedianFilter;
    EMAFilter<float> tempEmaFilter;
    MedianFilter<float> humidityMedianFilter;
    EMAFilter<float> humidityEmaFilter;
};

#endif // SENSORS_H