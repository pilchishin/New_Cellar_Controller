#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_HTU21DF.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "Types.h"
#include "Config.h" // Предполагается, что здесь задан пин ONE_WIRE_BUS
#include "MathUtils.h"

class SensorManager {
private:
    // Объекты библиотек для работы с датчиками
    Adafruit_BME280 bme;
    Adafruit_HTU21DF htu;
    OneWire oneWire;
    DallasTemperature dsSensor;

    // Структуры для хранения финальных (отфильтрованных) данных
    SensorData insideData;  // Данные внутри (BME280)
    SensorData outsideData; // Данные снаружи (HTU21D)
    float controlTemp;      // Контрольная температура подвала (DS18B20)

    // Объекты фильтров (Медиана N=3 + EMA alpha=0.2)
    // Создаем отдельный фильтр для каждого физического параметра
    Filter filterBmeTemp;
    Filter filterBmeHum;
    Filter filterHtuTemp;
    Filter filterHtuHum;
    Filter filterDsTemp;

    // Флаги состояния датчиков (исправен/неисправен)
    bool bmeValid;
    bool htuValid;
    bool dsValid;

    // Калибровочные коэффициенты
    CalibrationData calib;

public:
    // Конструктор: инициализируем OneWire пином из Config.h
    SensorManager();

    // Инициализация шин и самих датчиков
    void init();

    // Главный метод опроса, вызывается каждые 10 секунд из loop()
    void update();

    // Возвращает код ошибки, если что-то пошло не так
    ErrorCode checkErrors();

    // Геттеры для получения актуальных данных контроллером
    SensorData getInside() const { return insideData; }
    SensorData getOutside() const { return outsideData; }
    float getControlTemp() const { return controlTemp; }

    void setCalibration(const CalibrationData& data) { calib = data; }
};

#endif