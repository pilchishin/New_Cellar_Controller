/**
 * @file SensorManager.cpp
 * @brief Реализация управления датчиками, обработки данных и восстановления I2C.
 */

#include "SensorManager.h"

/**
 * @brief Конструктор SensorManager.
 * Инициализирует объекты шин, фильтры и структуры данных начальными значениями.
 */
SensorManager::SensorManager()
    : one_wire_(ONE_WIRE_BUS),
      ds_sensor_(&one_wire_),
      control_temp_(0.0f),
      filter_bme_temp_(kFilterEmaAlpha),
      filter_bme_hum_(kFilterEmaAlpha),
      filter_htu_temp_(kFilterEmaAlpha),
      filter_htu_hum_(kFilterEmaAlpha),
      filter_ds_temp_(kFilterEmaAlpha),
      bme_stat_({false, 0, 0, 0}),
      htu_stat_({false, 0, 0, 0}),
      ds_stat_({false, 0, 0, 0}),
      ds_request_ts_(0),
      i2c_error_count_(0) {
  inside_data_ = {0, 0, 0, 0, false};
  outside_data_ = {0, 0, 0, 0, false};
  calib_ = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
}

/**
 * @brief Начальная инициализация датчиков.
 */
void SensorManager::Init() {
#ifdef DEBUG
  Serial.println(F("Init Sensors..."));
#endif

  // Установка таймаута I2C (3мс) для предотвращения бесконечного ожидания при сбоях на линии.
  Wire.setWireTimeout(3000, true);

  InitBme();
  InitHtu();
  InitDs();
}

/**
 * @brief Инициализация датчика BME280 (внутренний).
 */
void SensorManager::InitBme() {
  if (bme_.begin(BME280_ADDR)) {
    bme_stat_.valid = true;
    bme_stat_.retries = 0;
    // Сбрасываем фильтры, чтобы старые (возможно ошибочные) данные не влияли на новые замеры.
    filter_bme_temp_.Invalidate();
    filter_bme_hum_.Invalidate();
    // Настройка: нормальный режим, минимальная передискретизация для экономии энергии и точности.
    bme_.setSampling(Adafruit_BME280::MODE_NORMAL, Adafruit_BME280::SAMPLING_X1,
                     Adafruit_BME280::SAMPLING_X1, Adafruit_BME280::SAMPLING_X1,
                     Adafruit_BME280::FILTER_OFF);
#ifdef DEBUG
    Serial.println(F("BME280 Init OK"));
#endif
  } else {
    bme_stat_.valid = false;
#ifdef DEBUG
    Serial.println(F("BME280 Init FAIL"));
#endif
  }
}

/**
 * @brief Инициализация датчика HTU21D (уличный).
 */
void SensorManager::InitHtu() {
  if (htu_.begin()) {
    htu_stat_.valid = true;
    htu_stat_.retries = 0;
    filter_htu_temp_.Invalidate();
    filter_htu_hum_.Invalidate();
#ifdef DEBUG
    Serial.println(F("HTU21D Init OK"));
#endif
  } else {
    htu_stat_.valid = false;
#ifdef DEBUG
    Serial.println(F("HTU21D Init FAIL"));
#endif
  }
}

/**
 * @brief Инициализация датчика DS18B20 (контрольный).
 */
void SensorManager::InitDs() {
  ds_sensor_.begin();
  if (ds_sensor_.getDeviceCount() > 0) {
    ds_stat_.valid = true;
    ds_stat_.retries = 0;
    filter_ds_temp_.Invalidate();
    // Максимальное разрешение (12 бит) для высокой точности.
    ds_sensor_.setResolution(12);
    // Асинхронный режим: не ждем 750мс внутри библиотеки.
    ds_sensor_.setWaitForConversion(false);
    ds_sensor_.requestTemperatures();
    ds_request_ts_ = millis();
#ifdef DEBUG
    Serial.println(F("DS18B20 Init OK"));
#endif
  } else {
    ds_stat_.valid = false;
#ifdef DEBUG
    Serial.println(F("DS18B20 Init FAIL"));
#endif
  }
}

/**
 * @brief Принудительная деактивация датчика BME280 при критических ошибках.
 */
void SensorManager::InvalidateBme() {
  bme_stat_.valid = false;
  bme_stat_.retries = 0;
  bme_stat_.stability_count = 0;
  bme_error_count_ = 0;
  inside_data_.valid = false;
  filter_bme_temp_.Invalidate();
  filter_bme_hum_.Invalidate();
}

/**
 * @brief Принудительная деактивация датчика HTU21D при критических ошибках.
 */
void SensorManager::InvalidateHtu() {
  htu_stat_.valid = false;
  htu_stat_.retries = 0;
  htu_stat_.stability_count = 0;
  htu_error_count_ = 0;
  outside_data_.valid = false;
  filter_htu_temp_.Invalidate();
  filter_htu_hum_.Invalidate();
}

/**
 * @brief Цикл обновления данных всех датчиков.
 */
void SensorManager::Update() {
  bool i2c_any_success = false; // Флаг того, что хотя бы одно устройство I2C ответило в этом цикле.
  unsigned long now = millis();

  // --- 1. ОБРАБОТКА BME280 (ВНУТРИ) ---

  // Попытка восстановить датчик, если он помечен как неисправный.
  if (!bme_stat_.valid && bme_stat_.retries < kSensorMaxRetries) {
    if (now - bme_stat_.lastRetry >= kSensorRetryInterval) {
      bme_stat_.lastRetry = now;
      bme_stat_.retries++;
#ifdef DEBUG
      Serial.print(F("BME280 Retry "));
      Serial.println(bme_stat_.retries);
#endif
      InitBme();
      if (bme_stat_.valid) i2c_any_success = true;
    }
  }

  if (!bme_stat_.valid && bme_stat_.retries >= kSensorMaxRetries) {
    unsigned long now_ms = millis();
    if (now_ms - bme_dead_retry_ts_ >= kSensorDeadRetryInterval) {
      bme_dead_retry_ts_ = now_ms;
      bme_stat_.retries = 0; // grant a fresh retry budget
#ifdef DEBUG
      Serial.println(F("BME280: dead-sensor hourly re-probe triggered."));
#endif
    }
  }

  if (bme_stat_.valid) {
    float raw_temp = bme_.readTemperature();
    float raw_hum = bme_.readHumidity();

    bool is_nan = isnan(raw_temp) || isnan(raw_hum);
    bool is_in_range = raw_temp >= kRawTempMin && raw_temp <= kRawTempMax &&
                       raw_hum >= kRawHumMin && raw_hum <= kRawHumMax;
    bool is_plausible = !is_nan && is_in_range;

    if (is_plausible) {
      i2c_any_success = true;
      bme_error_count_ = 0;

      // Инициализация или обновление фильтров.
      float filtered_temp, filtered_hum;
      if (!filter_bme_temp_.IsInitialized()) {
        filter_bme_temp_.Reset(raw_temp);
        filtered_temp = raw_temp;
      } else {
        filtered_temp = filter_bme_temp_.Update(raw_temp);
      }

      if (!filter_bme_hum_.IsInitialized()) {
        filter_bme_hum_.Reset(raw_hum);
        filtered_hum = raw_hum;
      } else {
        filtered_hum = filter_bme_hum_.Update(raw_hum);
      }

      // Сохранение результатов с учетом программной калибровки.
      inside_data_.temp = filtered_temp + calib_.bmeTempOffset;
      inside_data_.rh = filtered_hum + calib_.bmeHumOffset;

      // Расчет производных климатических параметров.
      inside_data_.ah = climate_math::CalculateAH(inside_data_.temp, inside_data_.rh);
      inside_data_.dewpoint = climate_math::CalculateDewPoint(inside_data_.temp, inside_data_.rh);
      inside_data_.valid = true;

      if (bme_stat_.stability_count < kSensorStabilityThreshold) {
        bme_stat_.stability_count++;
      }
    } else {
      // Данные недостоверны (NaN или вне диапазона): используем счетчик ошибок для толерантности.
      bme_error_count_++;
      if (bme_error_count_ >= kI2cMaxErrors) {
        InvalidateBme();
      } else {
        inside_data_.valid = false;
      }
#ifdef DEBUG
      Serial.println(is_nan ? F("BME280: NaN read!") : F("BME280: Data out of range!"));
#endif
    }
  }

  // --- 2. ОБРАБОТКА HTU21D (УЛИЦА) ---

  if (!htu_stat_.valid && htu_stat_.retries < kSensorMaxRetries) {
    if (now - htu_stat_.lastRetry >= kSensorRetryInterval) {
      htu_stat_.lastRetry = now;
      htu_stat_.retries++;
#ifdef DEBUG
      Serial.print(F("HTU21D Retry "));
      Serial.println(htu_stat_.retries);
#endif
      InitHtu();
      if (htu_stat_.valid) i2c_any_success = true;
    }
  }

  if (!htu_stat_.valid && htu_stat_.retries >= kSensorMaxRetries) {
    unsigned long now_ms = millis();
    if (now_ms - htu_dead_retry_ts_ >= kSensorDeadRetryInterval) {
      htu_dead_retry_ts_ = now_ms;
      htu_stat_.retries = 0;
#ifdef DEBUG
      Serial.println(F("HTU21D: dead-sensor hourly re-probe triggered."));
#endif
    }
  }

  if (htu_stat_.valid) {
    float raw_temp = htu_.readTemperature();
    float raw_hum = htu_.readHumidity();

    bool is_nan = isnan(raw_temp) || isnan(raw_hum);
    bool is_in_range = raw_temp >= kRawTempMin && raw_temp <= kRawTempMax &&
                       raw_hum >= kRawHumMin && raw_hum <= kRawHumMax;
    bool is_plausible = !is_nan && is_in_range;

    if (is_plausible) {
      i2c_any_success = true;
      htu_error_count_ = 0;

      float filtered_temp, filtered_hum;
      if (!filter_htu_temp_.IsInitialized()) {
        filter_htu_temp_.Reset(raw_temp);
        filtered_temp = raw_temp;
      } else {
        filtered_temp = filter_htu_temp_.Update(raw_temp);
      }

      if (!filter_htu_hum_.IsInitialized()) {
        filter_htu_hum_.Reset(raw_hum);
        filtered_hum = raw_hum;
      } else {
        filtered_hum = filter_htu_hum_.Update(raw_hum);
      }

      outside_data_.temp = filtered_temp + calib_.htuTempOffset;
      outside_data_.rh = filtered_hum + calib_.htuHumOffset;
      outside_data_.ah = climate_math::CalculateAH(outside_data_.temp, outside_data_.rh);
      outside_data_.dewpoint = climate_math::CalculateDewPoint(outside_data_.temp, outside_data_.rh);
      outside_data_.valid = true;

      if (htu_stat_.stability_count < kSensorStabilityThreshold) {
        htu_stat_.stability_count++;
      }
    } else {
      htu_error_count_++;
      if (htu_error_count_ >= kI2cMaxErrors) {
        InvalidateHtu();
      } else {
        outside_data_.valid = false;
      }
#ifdef DEBUG
      Serial.println(is_nan ? F("HTU21D: NaN read!") : F("HTU21D: Data out of range!"));
#endif
    }
  }

  // --- 3. ОБРАБОТКА DS18B20 (КОНТРОЛЬ) ---

  if (!ds_stat_.valid && ds_stat_.retries < kSensorMaxRetries) {
    if (now - ds_stat_.lastRetry >= kSensorRetryInterval) {
      ds_stat_.lastRetry = now;
      ds_stat_.retries++;
#ifdef DEBUG
      Serial.print(F("DS18B20 Retry "));
      Serial.println(ds_stat_.retries);
#endif
      InitDs();
    }
  }

  // Проверка готовности данных (минимум 750мс после запроса).
  if (ds_stat_.valid && (millis() - ds_request_ts_ >= 750)) {
    float raw_ds_temp = ds_sensor_.getTempCByIndex(0);

    if (raw_ds_temp == DEVICE_DISCONNECTED_C) {
      ds_stat_.valid = false;
      ds_stat_.stability_count = 0;
      filter_ds_temp_.Invalidate();
#ifdef DEBUG
      Serial.println(F("DS18B20: Disconnected!"));
#endif
    } else if (raw_ds_temp == 85.0f) {
      // Значение 85.0C игнорируется, так как это код ошибки сброса питания датчика.
#ifdef DEBUG
      Serial.println(F("DS18B20: 85.0C ignored."));
#endif
    } else {
      float filtered_ds;
      if (!filter_ds_temp_.IsInitialized()) {
        filter_ds_temp_.Reset(raw_ds_temp);
        filtered_ds = raw_ds_temp;
      } else {
        filtered_ds = filter_ds_temp_.Update(raw_ds_temp);
      }
      control_temp_ = filtered_ds + calib_.dsTempOffset;

      if (ds_stat_.stability_count < kSensorStabilityThreshold) {
        ds_stat_.stability_count++;
      }
    }

    // Запуск нового цикла измерения.
    ds_sensor_.requestTemperatures();
    ds_request_ts_ = millis();
  }

  // --- 4. МОНИТОРИНГ I2C ---

  if (i2c_any_success) {
    i2c_error_count_ = 0; // Сброс при любом успешном обмене.
  } else {
    // Если ни один I2C датчик не ответил, инкрементируем счетчик ошибок.
    if (i2c_error_count_ < kI2cErrorMax) {
      i2c_error_count_++;
    }
#ifdef DEBUG
    Serial.print(F("I2C Error Count: "));
    Serial.println(i2c_error_count_);
#endif
  }
}

/**
 * @brief Диагностика ошибок датчиков и расхождений данных.
 */
ErrorCode SensorManager::CheckErrors() {
  if (!bme_stat_.valid) return ErrorCode::kSensorBmeFail;
  if (!htu_stat_.valid) return ErrorCode::kSensorHtuFail;
  if (!ds_stat_.valid) return ErrorCode::kSensorDsFail;

  // Проверка расхождения температур выполняется только после стабилизации показаний.
  bool is_stable = (bme_stat_.stability_count >= kSensorStabilityThreshold &&
                    ds_stat_.stability_count >= kSensorStabilityThreshold);

  if (is_stable && fabsf(inside_data_.temp - control_temp_) > kSensorDiffMax) {
#ifdef DEBUG
    Serial.print(F("Temp Mismatch! BME: "));
    Serial.print(inside_data_.temp);
    Serial.print(F(" DS: "));
    Serial.println(control_temp_);
#endif
    return ErrorCode::kTempMismatch;
  }

  return ErrorCode::kNone;
}

/**
 * @brief Процедура "жесткого" восстановления шины I2C.
 */
void SensorManager::Recover() {
  Wire.end();
  delay(10);
  // Очистка SDA линии путем bit-bang тактирования SCL.
  i2c_utils::RecoverBus(A4, A5);
  Wire.begin();
  Wire.setWireTimeout(3000, true);

  // Сброс счетчика и попытка переинициализации датчиков.
  i2c_error_count_ = 0;
  bme_error_count_ = 0;
  htu_error_count_ = 0;
  bme_dead_retry_ts_ = 0;
  htu_dead_retry_ts_ = 0;
  InitBme();
  InitHtu();
  InitDs();
}
