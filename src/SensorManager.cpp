#include "SensorManager.h"

// Конструктор инициализирует шину 1-Wire и передает ее в DallasTemperature
SensorManager::SensorManager() 
    : oneWire(ONE_WIRE_BUS), dsSensor(&oneWire),
      bmeValid(false), htuValid(false), dsValid(false),
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

    // Инициализация BME280 (внутренний климат). Адрес может быть 0x76 или 0x77
    if (!bme.begin(0x76)) {
        #ifdef DEBUG
        Serial.println(F("BME280 Init Failed!"));
        #endif
        bmeValid = false;
    } else {
        bmeValid = true;
        // Настройка BME280 для метеостанции (рекомендации из даташита)
        bme.setSampling(Adafruit_BME280::MODE_NORMAL,
                        Adafruit_BME280::SAMPLING_X1,  // Температура
                        Adafruit_BME280::SAMPLING_X1,  // Давление (не используем, но нужно для работы)
                        Adafruit_BME280::SAMPLING_X1,  // Влажность
                        Adafruit_BME280::FILTER_OFF);
    }

    // Инициализация HTU21D (улица)
    if (!htu.begin()) {
        #ifdef DEBUG
        Serial.println(F("HTU21D Init Failed!"));
        #endif
        htuValid = false;
    } else {
        htuValid = true;
    }

    // Инициализация DS18B20 (контрольный датчик)
    dsSensor.begin();
    // Проверяем, найден ли хотя бы один датчик на шине
    if (dsSensor.getDeviceCount() == 0) {
        #ifdef DEBUG
        Serial.println(F("DS18B20 Init Failed!"));
        #endif
        dsValid = false;
    } else {
        // Устанавливаем разрешение 12 бит (0.0625°C) для высокой точности
        dsSensor.setResolution(12);
        // Отключаем ожидание конверсии, чтобы не блокировать цикл (non-blocking mode)
        dsSensor.setWaitForConversion(false);
        dsValid = true;
        // Отправляем первую команду на замер температуры
        dsSensor.requestTemperatures(); 
    }
}

void SensorManager::update() {
    bool i2cSuccess = false;

    // 1. ОПРОС И ФИЛЬТРАЦИЯ BME280 (ПОМЕЩЕНИЕ)
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

    // Обработка ошибок I2C
    if (i2cSuccess) {
        i2cErrorCount = 0; // Сброс при успешном чтении хотя бы одного датчика
    } else if (bmeValid || htuValid) {
        // Если датчики должны работать, но чтение не удалось
        i2cErrorCount++;
        #ifdef DEBUG
        Serial.print(F("I2C Error Count: ")); Serial.println(i2cErrorCount);
        #endif
    }

    // 3. ОПРОС И ФИЛЬТРАЦИЯ DS18B20 (КОНТРОЛЬ ПОДВАЛА)
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