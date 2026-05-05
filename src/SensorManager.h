#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_HTU21DF.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "Types.h"
#include "config.h"
#include "MathUtils.h"

/**
 * @file SensorManager.h
 * @brief Определение класса SensorManager для управления датчиками и сбора данных.
 */

/**
 * @struct SensorStatus
 * @brief Хранит текущее состояние здоровья и статистику восстановления датчика.
 */
struct SensorStatus {
  bool valid;              ///< Флаг исправности датчика (успешное чтение и валидация).
  uint8_t retries;         ///< Текущее количество попыток повторной инициализации.
  unsigned long lastRetry; ///< Время последней попытки ретрая (мс).
  uint8_t stability_count; ///< Счетчик последовательных валидных чтений для стабилизации фильтров.
};

/**
 * @class SensorManager
 * @brief Центральный модуль управления всеми датчиками системы (BME280, HTU21D, DS18B20).
 *
 * Класс отвечает за:
 * - Периодический опрос аппаратных датчиков.
 * - Фильтрацию сырых данных (Медианный фильтр + EMA).
 * - Расчет производных параметров (абсолютная влажность, точка росы).
 * - Диагностику аппаратных сбоев и расхождений показаний.
 * - Автоматическое восстановление I2C шины при зависаниях.
 */
class SensorManager {
 private:
  // --- Объекты библиотек для работы с оборудованием ---
  Adafruit_BME280 bme_;        ///< Датчик параметров внутри погреба (шина I2C).
  Adafruit_HTU21DF htu_;       ///< Датчик параметров на улице (шина I2C).
  OneWire one_wire_;           ///< Шина 1-Wire для работы с DS18B20.
  DallasTemperature ds_sensor_; ///< Контрольный датчик температуры DS18B20.

  // --- Контейнеры для хранения обработанных данных ---
  SensorData inside_data_;   ///< Отфильтрованные результаты BME280 (внутренние).
  SensorData outside_data_;  ///< Отфильтрованные результаты HTU21D (уличные).
  float control_temp_;       ///< Отфильтрованный результат DS18B20 (контрольный).

  // --- Объекты фильтров (Window 3 + alpha 0.2) ---
  EmaMedianFilter filter_bme_temp_; ///< Фильтр температуры помещения.
  EmaMedianFilter filter_bme_hum_;  ///< Фильтр влажности помещения.
  EmaMedianFilter filter_htu_temp_; ///< Фильтр температуры улицы.
  EmaMedianFilter filter_htu_hum_;  ///< Фильтр влажности улицы.
  EmaMedianFilter filter_ds_temp_;  ///< Фильтр контрольной температуры.

  // --- Статусы жизненного цикла датчиков ---
  SensorStatus bme_stat_;       ///< Состояние датчика BME280.
  SensorStatus htu_stat_;       ///< Состояние датчика HTU21D.
  SensorStatus ds_stat_;        ///< Состояние датчика DS18B20.
  unsigned long ds_request_ts_; ///< Время последнего запроса конверсии для DS18B20 (мс).

  // --- Внутренние методы инициализации ---
  void InitBme(); ///< Конфигурация и запуск BME280.
  void InitHtu(); ///< Конфигурация и запуск HTU21D.
  void InitDs();  ///< Поиск и настройка датчика на шине 1-Wire.

  // --- Диагностика I2C шины ---
  uint16_t i2c_error_count_;       ///< Счетчик последовательных неудачных транзакций по I2C.
  const uint8_t kI2cMaxErrors = 3; ///< Порог ошибок, после которого инициируется восстановление шины.
  static constexpr uint16_t kI2cErrorMax = 1000; ///< Максимальное значение счетчика ошибок (защита от переполнения).

  CalibrationData calib_; ///< Смещения для программной калибровки показаний.

 public:
  /**
   * @brief Конструктор. Настраивает объекты шин и инициализирует фильтры.
   */
  SensorManager();

  /**
   * @brief Инициализация всех шин и датчиков.
   * Вызывается один раз при старте или при глубоком сбросе системы.
   */
  void Init();

  /**
   * @brief Основной метод обновления данных.
   * Выполняет опрос датчиков, фильтрацию и расчет климатических параметров.
   */
  void Update();

  /**
   * @brief Диагностика состояния датчиков и достоверности данных.
   * Проверяет наличие аппаратных ошибок и расхождение температур между датчиками.
   * @return ErrorCode Код обнаруженной ошибки или kNone.
   */
  ErrorCode CheckErrors();

  /**
   * @brief Процедура аварийного восстановления I2C шины.
   * Пытается "разморозить" шину методом bit-bang и переинициализирует датчики.
   */
  void Recover();

  /**
   * @brief Возвращает актуальные данные внутреннего датчика.
   */
  SensorData GetInside() const { return inside_data_; }

  /**
   * @brief Возвращает актуальные данные уличного датчика.
   */
  SensorData GetOutside() const { return outside_data_; }

  /**
   * @brief Возвращает актуальную контрольную температуру.
   */
  float GetControlTemp() const { return control_temp_; }

  /**
   * @brief Проверяет, находится ли I2C шина в состоянии критического сбоя.
   * @return true Если количество ошибок превысило допустимый порог.
   */
  bool IsI2cFailing() const { return i2c_error_count_ >= kI2cMaxErrors; }

  /**
   * @brief Устанавливает новые калибровочные коэффициенты.
   * @param data Структура со смещениями для всех параметров.
   */
  void SetCalibration(const CalibrationData& data) { calib_ = data; }
};

#endif
