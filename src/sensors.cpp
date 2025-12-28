#include "sensors.h"

SensorManager::SensorManager() {
    // Конструктор
}

void SensorManager::init() {
    // Инициализация сенсоров
}

void SensorManager::readSensors() {
    // Чтение данных с сенсоров
}

float SensorManager::getTemperature() {
    return 0.0f;
}

float SensorManager::getHumidity() {
    return 0.0f;
}

float SensorManager::getOzoneLevel() {
    return 0.0f;
}

bool SensorManager::isTemperatureValid() {
    return false;
}

bool SensorManager::isHumidityValid() {
    return false;
}

bool SensorManager::isOzoneValid() {
    return false;
}