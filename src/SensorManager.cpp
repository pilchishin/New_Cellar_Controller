#include "SensorManager.h"

// Конструктор инициализирует шину 1-Wire и передает ее в DallasTemperature
SensorManager::SensorManager()
    : one_wire_(ONE_WIRE_BUS),
      ds_sensor_(&one_wire_),
      bme_stat_({false, 0, 0}),
      htu_stat_({false, 0, 0}),
      ds_stat_({false, 0, 0}),
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
    bme_stat_.valid = true;
    bme_stat_.retries = 0;
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
  if (htu_.begin()) {
    htu_stat_.valid = true;
    htu_stat_.retries = 0;
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
  ds_sensor_.begin();
  if (ds_sensor_.getDeviceCount() > 0) {
    ds_stat_.valid = true;
    ds_stat_.retries = 0;
    ds_sensor_.setResolution(12);
    ds_sensor_.setWaitForConversion(false);
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

  // Вспомогательная лямбда для проверки ретраев (С++11)
  auto check_retry = [&](SensorStatus& stat, void (SensorManager::*init_func)(),
                         const char* name) {
    if (!stat.valid && stat.retries < kMaxRetries) {
      if (now - stat.lastRetry >= kRetryInterval) {
        stat.lastRetry = now;
        stat.retries++;
#ifdef DEBUG
        Serial.print(name);
        Serial.print(F(" Retry "));
        Serial.println(stat.retries);
#endif
        (this->*init_func)();
      }
    }
  };

  // 1. ПРОВЕРКА РЕТРАЕВ
  check_retry(bme_stat_, &SensorManager::InitBme, "BME280");
  check_retry(htu_stat_, &SensorManager::InitHtu, "HTU21D");
  check_retry(ds_stat_, &SensorManager::InitDs, "DS18B20");

  // 2. ОПРОС И ФИЛЬТРАЦИЯ BME280 (ПОМЕЩЕНИЕ)
  if (bme_stat_.valid) {
    float raw_temp = bme_.readTemperature();
    float raw_hum = bme_.readHumidity();

    if (isnan(raw_temp) || isnan(raw_hum)) {
      bme_stat_.valid = false;
      inside_data_.valid = false;
    } else {
      inside_data_.temp = filter_bme_temp_.Update(raw_temp) + calib_.bmeTempOffset;
      inside_data_.rh = filter_bme_hum_.Update(raw_hum) + calib_.bmeHumOffset;
      inside_data_.ah = climate_math::CalculateAH(inside_data_.temp, inside_data_.rh);
      inside_data_.dewpoint =
          climate_math::CalculateDewPoint(inside_data_.temp, inside_data_.rh);
      inside_data_.valid = true;
      i2c_success = true;
    }
  }

  // 3. ОПРОС И ФИЛЬТРАЦИЯ HTU21D (УЛИЦА)
  if (htu_stat_.valid) {
    float raw_temp = htu_.readTemperature();
    float raw_hum = htu_.readHumidity();

    if (isnan(raw_temp) || isnan(raw_hum) || raw_hum > 100.0f) {
      htu_stat_.valid = false;
      outside_data_.valid = false;
    } else {
      outside_data_.temp = filter_htu_temp_.Update(raw_temp) + calib_.htuTempOffset;
      outside_data_.rh = filter_htu_hum_.Update(raw_hum) + calib_.htuHumOffset;
      outside_data_.ah =
          climate_math::CalculateAH(outside_data_.temp, outside_data_.rh);
      outside_data_.dewpoint =
          climate_math::CalculateDewPoint(outside_data_.temp, outside_data_.rh);
      outside_data_.valid = true;
      i2c_success = true;
    }
  }

  // Логика обнаружения полного отказа I2C-шины
  if (i2c_success) {
    i2c_error_count_ = 0;
  } else if (bme_stat_.valid || htu_stat_.valid) {
    i2c_error_count_++;
#ifdef DEBUG
    Serial.print(F("I2C Error Count: "));
    Serial.println(i2c_error_count_);
#endif
  }

  // 4. ОПРОС И ФИЛЬТРАЦИЯ DS18B20 (КОНТРОЛЬ ПОДВАЛА)
  if (ds_stat_.valid) {
    float raw_ds_temp = ds_sensor_.getTempCByIndex(0);
    if (raw_ds_temp == DEVICE_DISCONNECTED_C) {
      ds_stat_.valid = false;
    } else {
      control_temp_ = filter_ds_temp_.Update(raw_ds_temp) + calib_.dsTempOffset;
    }
    ds_sensor_.requestTemperatures();
  }
}

ErrorCode SensorManager::CheckErrors() {
  if (!bme_stat_.valid) return ErrorCode::kSensorBmeFail;
  if (!htu_stat_.valid) return ErrorCode::kSensorHtuFail;
  if (!ds_stat_.valid) return ErrorCode::kSensorDsFail;

  if (abs(inside_data_.temp - control_temp_) > kSensorDiffMax) {
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
  i2c_utils::RecoverBus(A4, A5);
  // Пробуем инициализировать датчики заново
  Init();
}