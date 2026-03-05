#include "SensorManager.h"

// Конструктор инициализирует шину 1-Wire и передает ее в DallasTemperature
SensorManager::SensorManager() 
    : oneWire(ONE_WIRE_BUS), dsSensor(&oneWire),
      bmeValid(false), htuValid(false), dsValid(false),
      bmeRetries(0), htuRetries(0), dsRetries(0),
      lastBmeRetry(0), lastHtuRetry(0), lastDsRetry(0),
      i2cErrorCount(0), controlTemp(0.0f) {
    // Начальные значения структур сбрасываем в нули/false
    insideData = {0, 0, 0, 0, false};
    outsideData = {0, 0, 0, 0, false};
    calib = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
}

void SensorManager::init() {
    #ifdef DEBUG
    Serial.println(F("Init Sensors..."));
    #endif

    initBme();
    initHtu();
    initDs();
}

void SensorManager::initBme() {
    if (bme.begin(0x76)) {
        bmeValid = true;
        bmeRetries = 0;
        bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                        Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::SAMPLING_X1,
                        Adafruit_BME280::FILTER_OFF);
        #ifdef DEBUG
        Serial.println(F("BME280 Init OK"));
        #endif
    } else {
        bmeValid = false;
        #ifdef DEBUG
        Serial.println(F("BME280 Init FAIL"));
        #endif
    }
}

void SensorManager::initHtu() {
    if (htu.begin()) {
        htuValid = true;
        htuRetries = 0;
        #ifdef DEBUG
        Serial.println(F("HTU21D Init OK"));
        #endif
    } else {
        htuValid = false;
        #ifdef DEBUG
        Serial.println(F("HTU21D Init FAIL"));
        #endif
    }
}

void SensorManager::initDs() {
    dsSensor.begin();
    if (dsSensor.getDeviceCount() > 0) {
        dsValid = true;
        dsRetries = 0;
        dsSensor.setResolution(12);
        dsSensor.setWaitForConversion(false);
        dsSensor.requestTemperatures();
        #ifdef DEBUG
        Serial.println(F("DS18B20 Init OK"));
        #endif
    } else {
        dsValid = false;
        #ifdef DEBUG
        Serial.println(F("DS18B20 Init FAIL"));
        #endif
    }
}

/**
 * @brief Главный цикл опроса датчиков и восстановления I2C шины.
 * Вызывается каждые 10 секунд. Опрашивает исправные датчики и пробует
 * переинициализировать неисправные.
 */
void SensorManager::update() {
    bool i2cSuccess = false; // Флаг успешного чтения хотя бы одного I2C устройства
    unsigned long now = millis();

    // 1. ОПРОС И ФИЛЬТРАЦИЯ BME280 (ПОМЕЩЕНИЕ)
    if (!bmeValid && bmeRetries < MAX_RETRIES) {
        if (now - lastBmeRetry >= RETRY_INTERVAL) {
            lastBmeRetry = now;
            bmeRetries++;
            #ifdef DEBUG
            Serial.print(F("BME280 Retry ")); Serial.println(bmeRetries);
            #endif
            initBme();
        }
    }

    if (bmeValid) {
        float rawTemp = bme.readTemperature();
        float rawHum = bme.readHumidity();

        // Проверка на NaN (ошибка чтения)
        if (isnan(rawTemp) || isnan(rawHum)) {
            bmeValid = false;
            insideData.valid = false;
        } else {
            // Пропускаем сырые данные через фильтры (Медиана -> EMA)
            insideData.temp = filterBmeTemp.update(rawTemp) + calib.bmeTempOffset;
            insideData.rh   = filterBmeHum.update(rawHum) + calib.bmeHumOffset;
            
            // Абсолютная влажность и точка росы считаются ТОЛЬКО по отфильтрованным данным
            insideData.ah       = ClimateMath::calculateAH(insideData.temp, insideData.rh);
            insideData.dewpoint = ClimateMath::calculateDewPoint(insideData.temp, insideData.rh);
            insideData.valid    = true;
            i2cSuccess = true;
        }
    }

    // 2. ОПРОС И ФИЛЬТРАЦИЯ HTU21D (УЛИЦА)
    if (!htuValid && htuRetries < MAX_RETRIES) {
        if (now - lastHtuRetry >= RETRY_INTERVAL) {
            lastHtuRetry = now;
            htuRetries++;
            #ifdef DEBUG
            Serial.print(F("HTU21D Retry ")); Serial.println(htuRetries);
            #endif
            initHtu();
        }
    }

    if (htuValid) {
        float rawTemp = htu.readTemperature();
        float rawHum = htu.readHumidity();

        if (isnan(rawTemp) || isnan(rawHum) || rawHum > 100.0f) { // HTU иногда выдает >100% при ошибках
            htuValid = false;
            outsideData.valid = false;
        } else {
            outsideData.temp = filterHtuTemp.update(rawTemp) + calib.htuTempOffset;
            outsideData.rh   = filterHtuHum.update(rawHum) + calib.htuHumOffset;
            
            outsideData.ah       = ClimateMath::calculateAH(outsideData.temp, outsideData.rh);
            outsideData.dewpoint = ClimateMath::calculateDewPoint(outsideData.temp, outsideData.rh);
            outsideData.valid    = true;
            i2cSuccess = true;
        }
    }

    // Логика обнаружения полного отказа I2C-шины:
    // Если ни одно I2C устройство не ответило, инкрементируем счетчик ошибок шины.
    if (i2cSuccess) {
        i2cErrorCount = 0; // Сброс счетчика при любом удачном чтении
    } else if (bmeValid || htuValid) {
        // Если датчики помечены как активные, но чтение не прошло
        i2cErrorCount++;
        #ifdef DEBUG
        Serial.print(F("I2C Error Count: ")); Serial.println(i2cErrorCount);
        #endif
    }

    // 3. ОПРОС И ФИЛЬТРАЦИЯ DS18B20 (КОНТРОЛЬ ПОДВАЛА)
    if (!dsValid && dsRetries < MAX_RETRIES) {
        if (now - lastDsRetry >= RETRY_INTERVAL) {
            lastDsRetry = now;
            dsRetries++;
            #ifdef DEBUG
            Serial.print(F("DS18B20 Retry ")); Serial.println(dsRetries);
            #endif
            initDs();
        }
    }

    if (dsValid) {
        // Читаем значение из памяти датчика (результат предыдущего запроса)
        float rawDsTemp = dsSensor.getTempCByIndex(0);
        
        if (rawDsTemp == DEVICE_DISCONNECTED_C) {
            dsValid = false;
        } else {
            controlTemp = filterDsTemp.update(rawDsTemp) + calib.dsTempOffset;
        }
        
        // Сразу запрашиваем новую конверсию для следующего цикла опроса (через 10 сек)
        // Так как setWaitForConversion(false), эта функция выполнится мгновенно
        dsSensor.requestTemperatures();
    }
}

ErrorCode SensorManager::checkErrors() {
    // Проверка отказов самих модулей (железная ошибка)
    if (!bmeValid) return ErrorCode::SENSOR_BME_FAIL;
    if (!htuValid) return ErrorCode::SENSOR_HTU_FAIL;
    if (!dsValid)  return ErrorCode::SENSOR_DS_FAIL;

    // Логическая проверка: рассинхронизация основного (BME280) и контрольного (DS18B20) датчиков
    // Если разница больше 2°C, возможно один из датчиков деградировал или врет
    if (abs(insideData.temp - controlTemp) > 2.0f) {
        #ifdef DEBUG
        Serial.print(F("Temp Mismatch! BME: ")); Serial.print(insideData.temp);
        Serial.print(F(" DS: ")); Serial.println(controlTemp);
        #endif
        return ErrorCode::TEMP_MISMATCH;
    }

    // Ошибок датчиков не обнаружено
    return ErrorCode::NONE;
}

void SensorManager::recover() {
    // Выполняем программный сброс шины (A4=SDA, A5=SCL на Arduino Nano)
    I2CUtils::recoverBus(A4, A5);
    // Пробуем инициализировать датчики заново
    init();
}