#ifndef SENSORS_H
#define SENSORS_H

#include "config.h"
#include <stdint.h>  // Для типов данных uint8_t, uint16_t, uint32_t
#include <Arduino.h> // Для функций и типов Arduino


// Определение для значений температуры при отключенном датчике
#ifndef DEVICE_DISCONNECTED_C
#define DEVICE_DISCONNECTED_C -127.0f
#endif

// Тип для адреса датчика DS18B20
typedef uint8_t DeviceAddress[8];

class SensorManager {
public:
    /**
     * @brief Конструктор SensorManager
     * Инициализирует все внутренние переменные и флаги
     */
    SensorManager();
    
    /**
     * @brief Инициализация всех датчиков
     * Выполняет инициализацию BME280, DS18B20 и HTU21D датчиков
     */
    void init();
    
    /**
     * @brief Чтение данных с датчиков
     * Считывает текущие значения температуры и влажности с доступных датчиков
     */
    void readSensors();
       
    /**
     * @brief Получение температуры с BME280
     * @return Температура в градусах Цельсия
     */
    float getBME280Temperature();
    
    /**
     * @brief Получение влажности с BME280
     * @return Влажность в процентах
     */
    float getBME280Humidity();
    
    /**
     * @brief Получение температуры с HTU21D
     * @return Температура в градусах Цельсия
     */
    float getHTU21DTemperature();
    
    /**
     * @brief Получение влажности с HTU21D
     * @return Влажность в процентах
     */
    float getHTU21DHumidity();
    
    /**
     * @brief Получение температуры с DS18B20
     * @return Температура в градусах Цельсия
     */
    float getDS18B20Temperature();
    
    /**
     * @brief Проверка валидности данных температуры с BME280
     * @return true если данные температуры с BME280 действительны, иначе false
     */
    bool isBME280TemperatureValid();
    
    /**
     * @brief Проверка валидности данных влажности с BME280
     * @return true если данные влажности с BME280 действительны, иначе false
     */
    bool isBME280HumidityValid();
    
    /**
     * @brief Проверка валидности данных температуры с HTU21D
     * @return true если данные температуры с HTU21D действительны, иначе false
     */
    bool isHTU21DTemperatureValid();
    
    /**
     * @brief Проверка валидности данных влажности с HTU21D
     * @return true если данные влажности с HTU21D действительны, иначе false
     */
    bool isHTU21DHumidityValid();
    
    /**
     * @brief Проверка валидности данных температуры с DS18B20
     * @return true если данные температуры с DS18B20 действительны, иначе false
     */
    bool isDS18B20TemperatureValid();
    
    
    
private:
    // Поля для управления датчиками
    // BME280 - датчик температуры, влажности и давления
    bool bme280Initialized;
    unsigned char bme280Address;
    
    // HTU21D - датчик температуры и влажности
    bool htu21dInitialized;
    unsigned char htu21dAddress;
    
    // DS18B20 - датчик температуры (1-Wire)
    bool ds18b20Initialized;
    DeviceAddress ds18b20Address;
    
    // Поля для хранения отдельных значений с датчиков
    float bme280Temperature;
    float bme280Humidity;
    float htu21dTemperature;
    float htu21dHumidity;
    float ds18b20Temperature;
    
    // Флаги валидности для каждого датчика
    bool bme280TempValid;
    bool bme280HumidityValid;
    bool htu21dTempValid;
    bool htu21dHumidityValid;
    bool ds18b20TempValid;
    
    // Методы для работы с DS18B20
    /**
     * @brief Инициализация датчика DS18B20
     *
     * Выполняет полную инициализацию датчика DS18B20 по протоколу 1-Wire:
     * - Сброс шины 1-Wire и проверка присутствия устройства
     * - Поиск и идентификация устройства DS18B20
     * - Установка разрешения измерения
     * - Валидация инициализации
     *
     * @return true в случае успешной инициализации, false в случае ошибки
     */
    bool initDS18B20();
    
    
    /**
     * @brief Чтение температуры с датчика DS18B20
     *
     * Выполняет чтение температуры с датчика DS18B20.
     * Включает запуск преобразования и получение результата.
     *
     * @param temp Переменная для хранения значения температуры
     * @return true в случае успешного чтения, false в случае ошибки
     */
    bool readDS18B20Temp(float& temp);
    
    /**
     * @brief Установка разрешения измерения DS18B20
     *
     * Устанавливает разрешение измерения температуры датчика DS18B20.
     * Максимальное разрешение 12 бит обеспечивает точность 0.0625°C.
     */
    void setDS18B20Resolution();
    
    /**
     * @brief Валидация датчика DS18B20
     *
     * Выполняет тестовое чтение температуры для проверки работоспособности датчика.
     * Используется для подтверждения корректной инициализации.
     *
     * @return true если датчик прошел валидацию, false в противном случае
     */
    bool validateDS18B20();
    
    /**
     * @brief Инициализация датчика HTU21D
     *
     * Выполняет полную инициализацию датчика влажности и температуры HTU21D через интерфейс I2C:
     * - Проверка соединения с устройством
     * - Сброс устройства при необходимости
     * - Настройка регистра конфигурации
     * - Установка разрешения измерений по умолчанию
     * - Отключение режима удержания при измерениях
     * - Инициализация таймингов для корректного взаимодействия с датчиком
     *
     * @return true в случае успешной инициализации, false в случае ошибки
     */
    bool initHTU21D();
    
    /**
     * @brief Сброс датчика HTU21D
     *
     * Выполняет программный сброс датчика HTU21D по I2C.
     *
     * @return true в случае успешного сброса, false в случае ошибки
     */
    bool resetHTU21D();
    
    /**
     * @brief Чтение температуры с датчика HTU21D
     *
     * Выполняет чтение температуры с датчика HTU21D.
     *
     * @param temp Переменная для хранения значения температуры
     * @return true в случае успешного чтения, false в случае ошибки
     */
    bool readHTU21DTemperature(float& temp);
    
    /**
     * @brief Чтение влажности с датчика HTU21D
     *
     * Выполняет чтение влажности с датчика HTU21D.
     *
     * @param humidity Переменная для хранения значения влажности
     * @return true в случае успешного чтения, false в случае ошибки
     */
    bool readHTU21DHumidity(float& humidity);
    
    /**
     * @brief Валидация датчика HTU21D
     *
     * Выполняет тестовое чтение температуры и влажности для проверки работоспособности датчика.
     * Используется для подтверждения корректной инициализации.
     *
     * @return true если датчик прошел валидацию, false в противном случае
     */
    bool validateHTU21D();
    
    /**
     * @brief Чтение регистра HTU21D
     *
     * Вспомогательная функция для чтения регистра датчика HTU21D.
     *
     * @param reg Адрес регистра для чтения
     * @param value Переменная для хранения значения регистра
     * @return true в случае успешного чтения, false в случае ошибки
     */
    bool readHTU21DRegister(uint8_t reg, uint8_t& value);
    
    /**
     * @brief Запись в регистр HTU21D
     *
     * Вспомогательная функция для записи в регистр датчика HTU21D.
     *
     * @param reg Адрес регистра для записи
     * @param value Значение для записи
     * @return true в случае успешной записи, false в случае ошибки
     */
    bool writeHTU21DRegister(uint8_t reg, uint8_t value);
    
    /**
     * @brief Проверка CRC для данных HTU21D
     *
     * Выполняет проверку CRC для полученных данных HTU21D.
     *
     * @param data Двухбайтовые данные для проверки
     * @param crc CRC-байт для проверки
     * @return true если CRC корректна, false в случае ошибки
     */
    bool checkCRC(uint16_t data, uint8_t crc);
};

#endif // SENSORS_H