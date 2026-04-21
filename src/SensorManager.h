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

/**
 * @struct SensorStatus
 * @brief Хранит текущее состояние здоровья и статистику восстановления датчика.
 */
struct SensorStatus {
  bool valid;              ///< Флаг исправности датчика (успешное чтение и валидация)
  uint8_t retries;         ///< Текущее количество попыток повторной инициализации
  unsigned long lastRetry; ///< Время последней попытки ретрая (мс)
  uint8_t stability_count; ///< Счетчик последовательных валидных чтений для стабилизации
};

/**
 * @class SensorManager
 * @brief Центральный модуль управления всеми датчиками системы (BME280, HTU21D, DS18B20).
 * Отвечает за опрос, фильтрацию Калмана, калибровку и диагностику аппаратных сбоев.
 */
class SensorManager {
 private:
  // --- Объекты библиотек для работы с оборудованием ---
  Adafruit_BME280 bme_;        ///< Датчик параметров внутри погреба (I2C)
  Adafruit_HTU21DF htu_;       ///< Датчик параметров на улице (I2C)
  OneWire one_wire_;           ///< Шина 1-Wire для DS18B20
  DallasTemperature ds_sensor_; ///< Контрольный датчик температуры (1-Wire)

  // --- Контейнеры для хранения отфильтрованных и откалиброванных данных ---
  SensorData inside_data_;   ///< Результаты BME280 (T, RH, AH, DewPoint)
  SensorData outside_data_;  ///< Результаты HTU21D (T, RH, AH, DewPoint)
  float control_temp_;       ///< Результат DS18B20 (T)

  // --- Объекты одномерных фильтров Калмана (1D Kalman Filter) ---
  KalmanFilter filter_bme_temp_; ///< Фильтр температуры помещения
  KalmanFilter filter_bme_hum_;  ///< Фильтр влажности помещения
  KalmanFilter filter_htu_temp_; ///< Фильтр температуры улицы
  KalmanFilter filter_htu_hum_;  ///< Фильтр влажности улицы
  KalmanFilter filter_ds_temp_;  ///< Фильтр контрольной температуры

  // --- Статусы жизненного цикла датчиков ---
  SensorStatus bme_stat_; ///< Состояние BME280
  SensorStatus htu_stat_; ///< Состояние HTU21D
  SensorStatus ds_stat_;  ///< Состояние DS18B20

  // --- Внутренние методы инициализации ---
  void InitBme(); ///< Настройка параметров BME280
  void InitHtu(); ///< Настройка параметров HTU21D
  void InitDs();  ///< Поиск и настройка DS18B20

  // --- Диагностика I2C шины ---
  uint8_t i2c_error_count_;        ///< Счетчик последовательных ошибок обмена по I2C
  const uint8_t kI2cMaxErrors = 3; ///< Порог ошибок для запуска процедуры восстановления

  CalibrationData calib_; ///< Хранилище калибровочных смещений

 public:
  SensorManager();

  /**
   * @brief Инициализация всех шин и датчиков.
   * Вызывается при старте и после сброса I2C-шины.
   */
  void Init();

  /**
   * @brief Основной метод опроса датчиков.
   * Вызывается по расписанию из главного цикла (обычно раз в 10 сек).
   */
  void Update();

  /**
   * @brief Диагностика состояния датчиков.
   * @return ErrorCode Код первой обнаруженной критической ошибки.
   */
  ErrorCode CheckErrors();

  /**
   * @brief Процедура аварийного восстановления I2C шины.
   * Выполняет Wire.end(), bit-bang SCL и полную ре-инициализацию.
   */
  void Recover();

  // --- Геттеры для получения актуальных данных ---
  SensorData GetInside() const { return inside_data_; }
  SensorData GetOutside() const { return outside_data_; }
  float GetControlTemp() const { return control_temp_; }

  /**
   * @brief Проверка состояния "зависания" I2C-шины.
   */
  bool IsI2cFailing() const { return i2c_error_count_ >= kI2cMaxErrors; }

  /**
   * @brief Применение новых калибровочных данных.
   */
  void SetCalibration(const CalibrationData& data) { calib_ = data; }
};

#endif