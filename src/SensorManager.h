#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_HTU21DF.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "Types.h"
#include "config.h" // Предполагается, что здесь задан пин ONE_WIRE_BUS
#include "MathUtils.h"

class SensorManager {
 private:
  // Объекты библиотек для работы с датчиками
  Adafruit_BME280 bme_;
  Adafruit_HTU21DF htu_;
  OneWire one_wire_;
  DallasTemperature ds_sensor_;

  // Структуры для хранения финальных (отфильтрованных) данных
  SensorData inside_data_;   // Данные внутри (BME280)
  SensorData outside_data_;  // Данные снаружи (HTU21D)
  float control_temp_;       // Контрольная температура подвала (DS18B20)

  // Объекты фильтров (Медиана N=3 + EMA alpha=0.2)
  Filter filter_bme_temp_;
  Filter filter_bme_hum_;
  Filter filter_htu_temp_;
  Filter filter_htu_hum_;
  Filter filter_ds_temp_;

  // Флаги состояния датчиков (исправен/неисправен)
  bool bme_valid_;
  bool htu_valid_;
  bool ds_valid_;

  // Параметры повторных попыток
  uint8_t bme_retries_;
  uint8_t htu_retries_;
  uint8_t ds_retries_;
  unsigned long last_bme_retry_;
  unsigned long last_htu_retry_;
  unsigned long last_ds_retry_;
  const uint8_t kMaxRetries = 3;
  const uint32_t kRetryInterval = 60000UL;

  // Методы инициализации конкретных датчиков
  void InitBme();
  void InitHtu();
  void InitDs();

  // Счетчики ошибок для I2C устройств
  uint8_t i2c_error_count_;
  const uint8_t kI2cMaxErrors = 3;

  // Калибровочные коэффициенты
  CalibrationData calib_;

 public:
  SensorManager();

  // Инициализация шин и самих датчиков
  void Init();

  // Главный метод опроса
  void Update();

  // Возвращает код ошибки
  ErrorCode CheckErrors();

  // Попытка восстановления после сбоя I2C
  void Recover();

  // Геттеры для получения актуальных данных контроллером
  SensorData GetInside() const { return inside_data_; }
  SensorData GetOutside() const { return outside_data_; }
  float GetControlTemp() const { return control_temp_; }
  bool IsI2cFailing() const { return i2c_error_count_ >= kI2cMaxErrors; }

  void SetCalibration(const CalibrationData& data) { calib_ = data; }
};

#endif