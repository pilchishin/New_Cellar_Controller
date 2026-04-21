#include "SensorManager.h"

SensorManager::SensorManager()
    : one_wire_(ONE_WIRE_BUS),
      ds_sensor_(&one_wire_),
      control_temp_(0.0f),
      // Настройка фильтров Калмана: Q (шум процесса), R (шум измерения)
      filter_bme_temp_(0.01f, 0.5f),
      filter_bme_hum_(0.05f, 2.0f),
      filter_htu_temp_(0.01f, 0.5f),
      filter_htu_hum_(0.05f, 2.0f),
      filter_ds_temp_(0.01f, 0.5f),
      // Инициализация статусов датчиков
      bme_stat_({false, 0, 0, 0}),
      htu_stat_({false, 0, 0, 0}),
      ds_stat_({false, 0, 0, 0}),
      i2c_error_count_(0) {
  // Сброс структур данных в безопасное состояние
  inside_data_ = {0, 0, 0, 0, false};
  outside_data_ = {0, 0, 0, 0, false};
  calib_ = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
}

void SensorManager::Init() {
#ifdef DEBUG
  Serial.println(F("Init Sensors..."));
#endif

  // Инициализация шины I2C и установка таймаута для предотвращения зависаний
  Wire.begin();
  Wire.setWireTimeout(3000, true);

  InitBme();
  InitHtu();
  InitDs();
}

void SensorManager::InitBme() {
  // Попытка инициализации BME280 по заданному адресу I2C
  if (bme_.begin(BME280_ADDR)) {
    bme_stat_.valid = true;
    bme_stat_.retries = 0;
    // Сброс фильтров Калмана при переподключении датчика
    filter_bme_temp_.Reset();
    filter_bme_hum_.Reset();
    // Конфигурация режима работы и передискретизации
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

void SensorManager::InitHtu() {
  // Попытка инициализации датчика HTU21D (улица)
  if (htu_.begin()) {
    htu_stat_.valid = true;
    htu_stat_.retries = 0;
    // Сброс фильтров Калмана при переподключении датчика
    filter_htu_temp_.Reset();
    filter_htu_hum_.Reset();
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

void SensorManager::InitDs() {
  // Поиск датчиков на шине 1-Wire
  ds_sensor_.begin();
  if (ds_sensor_.getDeviceCount() > 0) {
    ds_stat_.valid = true;
    ds_stat_.retries = 0;
    // Сброс фильтра Калмана
    filter_ds_temp_.Reset();
    // Настройка разрешения (12 бит = 0.0625°C)
    ds_sensor_.setResolution(12);
    // Не блокируем выполнение на время конвертации (750 мс для 12 бит)
    ds_sensor_.setWaitForConversion(false);
    // Запускаем первое преобразование
    ds_sensor_.requestTemperatures();
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
 * @brief Главный цикл опроса датчиков и восстановления I2C шины.
 */
void SensorManager::Update() {
  bool i2c_success = false;  // Флаг успешного чтения хотя бы одного I2C устройства
  unsigned long now = millis();

  // 1. ОПРОС И ФИЛЬТРАЦИЯ BME280 (ПОМЕЩЕНИЕ)
  if (!bme_stat_.valid && bme_stat_.retries < kSensorMaxRetries) {
    if (now - bme_stat_.lastRetry >= kSensorRetryInterval) {
      bme_stat_.lastRetry = now;
      bme_stat_.retries++;
#ifdef DEBUG
      Serial.print(F("BME280 Retry "));
      Serial.println(bme_stat_.retries);
#endif
      InitBme();
    }
  }

  // 1. Опрос внутреннего датчика (BME280)
  if (bme_stat_.valid) {
    float raw_temp = bme_.readTemperature();
    float raw_hum = bme_.readHumidity();

    // Проверка на корректность данных (NaN и границы физического диапазона)
    bool is_invalid = isnan(raw_temp) || isnan(raw_hum) ||
                      raw_temp < kRawTempMin || raw_temp > kRawTempMax ||
                      raw_hum < kRawHumMin || raw_hum > kRawHumMax;

    if (is_invalid) {
      bme_stat_.valid = false;
      inside_data_.valid = false;
      bme_stat_.stability_count = 0;
#ifdef DEBUG
      Serial.println(F("BME280: Raw data out of range or NaN!"));
#endif
    } else {
      // Применяем фильтрацию Калмана и вносим калибровочное смещение
      inside_data_.temp = filter_bme_temp_.Update(raw_temp) + calib_.bmeTempOffset;
      inside_data_.rh = filter_bme_hum_.Update(raw_hum) + calib_.bmeHumOffset;

      // Расчет производных параметров на основе отфильтрованных данных
      inside_data_.ah = climate_math::CalculateAH(inside_data_.temp, inside_data_.rh);
      inside_data_.dewpoint =
          climate_math::CalculateDewPoint(inside_data_.temp, inside_data_.rh);
      inside_data_.valid = true;
      i2c_success = true; // Фиксируем успех обмена по шине I2C

      // Инкремент счетчика стабильности до достижения порога
      if (bme_stat_.stability_count < kSensorStabilityThreshold) {
        bme_stat_.stability_count++;
      }
    }
  }

  // 2. Опрос внешнего датчика (HTU21D)
  if (!htu_stat_.valid && htu_stat_.retries < kSensorMaxRetries) {
    if (now - htu_stat_.lastRetry >= kSensorRetryInterval) {
      htu_stat_.lastRetry = now;
      htu_stat_.retries++;
#ifdef DEBUG
      Serial.print(F("HTU21D Retry "));
      Serial.println(htu_stat_.retries);
#endif
      InitHtu();
    }
  }

  if (htu_stat_.valid) {
    float raw_temp = htu_.readTemperature();
    float raw_hum = htu_.readHumidity();

    // Проверка на корректность данных
    bool is_invalid = isnan(raw_temp) || isnan(raw_hum) ||
                      raw_temp < kRawTempMin || raw_temp > kRawTempMax ||
                      raw_hum < kRawHumMin || raw_hum > kRawHumMax;

    if (is_invalid) {
      htu_stat_.valid = false;
      outside_data_.valid = false;
      htu_stat_.stability_count = 0;
#ifdef DEBUG
      Serial.println(F("HTU21D: Raw data out of range or NaN!"));
#endif
    } else {
      // Применяем фильтрацию Калмана и калибровку
      outside_data_.temp = filter_htu_temp_.Update(raw_temp) + calib_.htuTempOffset;
      outside_data_.rh = filter_htu_hum_.Update(raw_hum) + calib_.htuHumOffset;

      // Расчет AH и DewPoint
      outside_data_.ah =
          climate_math::CalculateAH(outside_data_.temp, outside_data_.rh);
      outside_data_.dewpoint =
          climate_math::CalculateDewPoint(outside_data_.temp, outside_data_.rh);
      outside_data_.valid = true;
      i2c_success = true;

      if (htu_stat_.stability_count < kSensorStabilityThreshold) {
        htu_stat_.stability_count++;
      }
    }
  }

  // 3. Опрос контрольного датчика подвала (DS18B20)
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

  if (ds_stat_.valid) {
    // Читаем результат предыдущего запроса (асинхронная модель)
    float raw_ds_temp = ds_sensor_.getTempCByIndex(0);

    if (raw_ds_temp == DEVICE_DISCONNECTED_C) {
      ds_stat_.valid = false;
      ds_stat_.stability_count = 0;
    } else {
      // Фильтрация и калибровка
      control_temp_ = filter_ds_temp_.Update(raw_ds_temp) + calib_.dsTempOffset;
      if (ds_stat_.stability_count < kSensorStabilityThreshold) {
        ds_stat_.stability_count++;
      }
    }

    // Сразу запрашиваем новое измерение для следующего цикла опроса (через 10 сек)
    ds_sensor_.requestTemperatures();
  }

  // Логика обнаружения полного отказа I2C-шины
  if (i2c_success) {
    i2c_error_count_ = 0;
  } else {
    i2c_error_count_++;
#ifdef DEBUG
    Serial.print(F("I2C Error Count: "));
    Serial.println(i2c_error_count_);
#endif
  }
}

ErrorCode SensorManager::CheckErrors() {
  /**
   * ПРОВЕРКА КРИТИЧЕСКИХ ОШИБОК
   * 1. Аппаратные отказы (отсутствие ответа на шине или NaN).
   * 2. Расхождение показаний двух независимых сенсоров температуры.
   */

  if (!bme_stat_.valid) return ErrorCode::kSensorBmeFail;
  if (!htu_stat_.valid) return ErrorCode::kSensorHtuFail;
  if (!ds_stat_.valid) return ErrorCode::kSensorDsFail;

  // Проверка расхождения температур BME и DS только после периода стабилизации фильтров.
  // Это исключает ложные срабатывания при старте системы или после сброса датчиков.
  bool is_stable = (bme_stat_.stability_count >= kSensorStabilityThreshold &&
                    ds_stat_.stability_count >= kSensorStabilityThreshold);

  // Если оба датчика в стабильном состоянии, сравниваем их показания для обнаружения дрейфа или перегрева.
  if (is_stable && abs(inside_data_.temp - control_temp_) > kSensorDiffMax) {
#ifdef DEBUG
    Serial.print(F("Temp Mismatch! BME: "));
    Serial.print(inside_data_.temp);
    Serial.print(F(" DS: "));
    Serial.println(control_temp_);
#endif
    return ErrorCode::kTempMismatch;
  }

  // Ошибок не обнаружено
  return ErrorCode::kNone;
}

void SensorManager::Recover() {
  /**
   * Полноценное восстановление шины I2C.
   * 1. Отключаем TWI периферию.
   * 2. Ждем стабилизации уровней.
   * 3. Bit-bang 9 тактов SCL для высвобождения SDA.
   * 4. Перезапуск TWI с защитным таймаутом.
   */
  Wire.end();
  delay(10);
  i2c_utils::RecoverBus(A4, A5);
  Wire.begin();
  Wire.setWireTimeout(3000, true);

  // Переинициализация всех датчиков после сброса шины
  Init();
}
