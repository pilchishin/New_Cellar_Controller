#ifndef CONFIG_H
#define CONFIG_H

// Определения конфигурации для контроллера погреба

// Пины сенсоров
#define TEMP_SENSOR_PIN A0
#define HUMIDITY_SENSOR_PIN A1
#define OZONE_SENSOR_PIN A2

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
#define LCD_ROWS 4
#define LCD_COLS 20

// Адреса EEPROM
#define EEPROM_SETTINGS_ADDR 0
#define EEPROM_CALIBRATION_ADDR 50

#endif // CONFIG_H