#include "climate.h"

ClimateController::ClimateController() {
    targetTemperature = 0.0f;
    targetHumidity = 0.0f;
    currentTemperature = 0.0f;
    currentHumidity = 0.0f;
    heaterEnabled = false;
    fanEnabled = false;
    sensorManager = nullptr;
}

void ClimateController::init() {
    // Инициализация контроллера климата
}

void ClimateController::update() {
    // Обновление состояния климат-контроллера
    if (sensorManager != nullptr) {
        currentTemperature = sensorManager->getFilteredTemperature();
        currentHumidity = sensorManager->getFilteredHumidity();
    }
}

void ClimateController::setTargetTemperature(float temp) {
    // Установка целевой температуры
    targetTemperature = temp;
}

void ClimateController::setTargetHumidity(float humidity) {
    // Установка целевой влажности
    targetHumidity = humidity;
}

float ClimateController::getTargetTemperature() {
    return targetTemperature;
}

float ClimateController::getTargetHumidity() {
    return targetHumidity;
}

void ClimateController::enableHeater() {
    // Включение нагревателя
    heaterEnabled = true;
}

void ClimateController::disableHeater() {
    // Выключение нагревателя
    heaterEnabled = false;
}

void ClimateController::enableFan() {
    // Включение вентилятора
    fanEnabled = true;
}

void ClimateController::disableFan() {
    // Выключение вентилятора
    fanEnabled = false;
}

bool ClimateController::isHeaterEnabled() {
    return heaterEnabled;
}

bool ClimateController::isFanEnabled() {
    return fanEnabled;
}

float ClimateController::getCurrentTemperature() {
    return currentTemperature;
}

float ClimateController::getCurrentHumidity() {
    return currentHumidity;
}

void ClimateController::setSensorManager(SensorManager* sensorMgr) {
    sensorManager = sensorMgr;
}
