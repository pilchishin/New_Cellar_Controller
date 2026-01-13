#ifndef CONFIG_H
#define CONFIG_H

// Определения конфигурации для контроллера погреба

// Пины и адреса датчиков
#define DS18B20_PIN 2  // Пин для DS18B20 по умолчанию
#define BME280_ADDRESS 0x76  // Основной адрес BME280
#define HTU21D_ADDRESS 0x40  // Адрес HTU21D

// Пины управления
#define FAN_PIN 2
#define HEATER_PIN 3
#define OZONE_PIN 4
#define UV_LAMP_PIN 5

// Параметры работы
#define TEMP_MIN 5
#define TEMP_MAX 15
#define HUMIDITY_MIN 70
#define HUMIDITY_MAX 85
#define OZONE_MIN 0.05
#define OZONE_MAX 0.20

// Тайминги
#define OZONE_INTERVAL 3600000  // 1 час в миллисекундах
#define UV_LAMP_DURATION 180000  // 30 минут в миллисекундах

// Конфигурация дисплея
#define LCD_ROWS 2
#define LCD_COLS 16

// Адреса EEPROM
#define EEPROM_SETTINGS_ADDR 0
#define EEPROM_CALIBRATION_ADDR 50

// Константы для датчиков
#define DS18B20_RESOLUTION 12  // Разрешение DS18B20 в битах
#define HTU21D_TEMP_RESOLUTION 14  // Разрешение температуры HTU21D в битах
#define HTU21D_HUM_RESOLUTION 12  // Разрешение влажности HTU21D в битах

// Тайминги для датчиков
#define HTU21D_TEMP_MEASUREMENT_TIME 50 // Время измерения температуры HTU21D в мс
#define HTU21D_HUM_MEASUREMENT_TIME 16   // Время измерения влажности HTU21D в мс
#define SENSOR_POLL_INTERVAL_MS 10000    // Интервал опроса датчиков в миллисекундах (10 секунд)
#define SENSOR_READ_RETRIES  3

 

#endif // CONFIG_H