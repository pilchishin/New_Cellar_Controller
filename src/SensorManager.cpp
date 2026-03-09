#include "SensorManager.h"

// Конструктор инициализирует шину 1-Wire и передает ее в DallasTemperature
SensorManager::SensorManager()
    : one_wire_(ONE_WIRE_BUS),
      ds_sensor_(&one_wire_),
      bme_valid_(false),
      htu_valid_(false),
      ds_valid_(false),
      bme_retries_(0),
      htu_retries_(0),
      ds_retries_(0),
      last_bme_retry_(0),
      last_htu_retry_(0),
      last_ds_retry_(0),
      i2c_error_count_(0),
      control_temp_(0.0f) {
  // Начальные значения структур сбрасываем в нули/false
  inside_data_ = {0, 0, 0, 0, false};
  outside_data_ = {0, 0, 0, 0, false};
  calib_ = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
}

void SensorManager::Init() {
#ifdef DEBUG
  Serial.println(F("Init Sensors..."));
#endif

  InitBme();
  InitHtu();
  InitDs();
}

void SensorManager::InitBme() {
  if (bme_.begin(0x76)) {
    bme_valid_ = true;
    bme_retries_ = 0;
    bme_.setSampling(Adafruit_BME280::MODE_NORMAL, Adafruit_BME280::SAMPLING_X1,
                     Adafruit_BME280::SAMPLING_X1, Adafruit_BME280::SAMPLING_X1,
                     Adafruit_BME280::FILTER_OFF);
#ifdef DEBUG
    Serial.println(F("BME280 Init OK"));
#endif
  } else {
    bme_valid_ = false;
#ifdef DEBUG
    Serial.println(F("BME280 Init FAIL"));
#endif
  }
}

void SensorManager::InitHtu() {
  if (htu_.begin()) {
    htu_valid_ = true;
    htu_retries_ = 0;
#ifdef DEBUG
    Serial.println(F("HTU21D Init OK"));
#endif
  } else {
    htu_valid_ = false;
#ifdef DEBUG
    Serial.println(F("HTU21D Init FAIL"));
#endif
  }
}

void SensorManager::InitDs() {
  ds_sensor_.begin();
  if (ds_sensor_.getDeviceCount() > 0) {
    ds_valid_ = true;
    ds_retries_ = 0;
    ds_sensor_.setResolution(12);
    ds_sensor_.setWaitForConversion(false);
    ds_sensor_.requestTemperatures();
#ifdef DEBUG
    Serial.println(F("DS18B20 Init OK"));
#endif
  } else {
    ds_valid_ = false;
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
  if (!bme_valid_ && bme_retries_ < kMaxRetries) {
    if (now - last_bme_retry_ >= kRetryInterval) {
      last_bme_retry_ = now;
      bme_retries_++;
#ifdef DEBUG
      Serial.print(F("BME280 Retry "));
      Serial.println(bme_retries_);
#endif
      InitBme();
    }
  }

  if (bme_valid_) {
    float raw_temp = bme_.readTemperature();
    float raw_hum = bme_.readHumidity();

    // Проверка на NaN (ошибка чтения)
    if (isnan(raw_temp) || isnan(raw_hum)) {
      bme_valid_ = false;
      inside_data_.valid = false;
    } else {
      // Пропускаем сырые данные через фильтры (Медиана -> EMA)
      inside_data_.temp = filter_bme_temp_.update(raw_temp) + calib_.bmeTempOffset;
      inside_data_.rh = filter_bme_hum_.update(raw_hum) + calib_.bmeHumOffset;

      // Абсолютная влажность и точка росы считаются ТОЛЬКО по отфильтрованным данным
      inside_data_.ah = ClimateMath::calculateAH(inside_data_.temp, inside_data_.rh);
      inside_data_.dewpoint =
          ClimateMath::calculateDewPoint(inside_data_.temp, inside_data_.rh);
      inside_data_.valid = true;
      i2c_success = true;
    }
  }

  // 2. ОПРОС И ФИЛЬТРАЦИЯ HTU21D (УЛИЦА)
  if (!htu_valid_ && htu_retries_ < kMaxRetries) {
    if (now - last_htu_retry_ >= kRetryInterval) {
      last_htu_retry_ = now;
      htu_retries_++;
#ifdef DEBUG
      Serial.print(F("HTU21D Retry "));
      Serial.println(htu_retries_);
#endif
      InitHtu();
    }
  }

  if (htu_valid_) {
    float raw_temp = htu_.readTemperature();
    float raw_hum = htu_.readHumidity();

    if (isnan(raw_temp) || isnan(raw_hum) ||
        raw_hum > 100.0f) {  // HTU иногда выдает >100% при ошибках
      htu_valid_ = false;
      outside_data_.valid = false;
    } else {
      outside_data_.temp = filter_htu_temp_.update(raw_temp) + calib_.htuTempOffset;
      outside_data_.rh = filter_htu_hum_.update(raw_hum) + calib_.htuHumOffset;

      outside_data_.ah =
          ClimateMath::calculateAH(outside_data_.temp, outside_data_.rh);
      outside_data_.dewpoint =
          ClimateMath::calculateDewPoint(outside_data_.temp, outside_data_.rh);
      outside_data_.valid = true;
      i2c_success = true;
    }
  }

  // Логика обнаружения полного отказа I2C-шины:
  if (i2c_success) {
    i2c_error_count_ = 0;  // Сброс счетчика при любом удачном чтении
  } else if (bme_valid_ || htu_valid_) {
    // Если датчики помечены как активные, но чтение не прошло
    i2c_error_count_++;
#ifdef DEBUG
    Serial.print(F("I2C Error Count: "));
    Serial.println(i2c_error_count_);
#endif
  }

  // 3. ОПРОС И ФИЛЬТРАЦИЯ DS18B20 (КОНТРОЛЬ ПОДВАЛА)
  if (!ds_valid_ && ds_retries_ < kMaxRetries) {
    if (now - last_ds_retry_ >= kRetryInterval) {
      last_ds_retry_ = now;
      ds_retries_++;
#ifdef DEBUG
      Serial.print(F("DS18B20 Retry "));
      Serial.println(ds_retries_);
#endif
      InitDs();
    }
  }

  if (ds_valid_) {
    // Читаем значение из памяти датчика (результат предыдущего запроса)
    float raw_ds_temp = ds_sensor_.getTempCByIndex(0);

    if (raw_ds_temp == DEVICE_DISCONNECTED_C) {
      ds_valid_ = false;
    } else {
      control_temp_ = filter_ds_temp_.update(raw_ds_temp) + calib_.dsTempOffset;
    }

    // Сразу запрашиваем новую конверсию для следующего цикла опроса (через 10 сек)
    ds_sensor_.requestTemperatures();
  }
}

ErrorCode SensorManager::CheckErrors() {
  // Проверка отказов самих модулей (железная ошибка)
  if (!bme_valid_) return ErrorCode::kSensorBmeFail;
  if (!htu_valid_) return ErrorCode::kSensorHtuFail;
  if (!ds_valid_) return ErrorCode::kSensorDsFail;

  // Логическая проверка: рассинхронизация основного (BME280) и контрольного (DS18B20) датчиков
  if (abs(inside_data_.temp - control_temp_) > 2.0f) {
#ifdef DEBUG
    Serial.print(F("Temp Mismatch! BME: "));
    Serial.print(inside_data_.temp);
    Serial.print(F(" DS: "));
    Serial.println(control_temp_);
#endif
    return ErrorCode::kTempMismatch;
  }

  // Ошибок датчиков не обнаружено
  return ErrorCode::kNone;
}

void SensorManager::Recover() {
  // Выполняем программный сброс шины (A4=SDA, A5=SCL на Arduino Nano)
  I2CUtils::recoverBus(A4, A5);
  // Пробуем инициализировать датчики заново
  Init();
}