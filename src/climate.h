#ifndef CLIMATE_H
#define CLIMATE_H

#include "config.h"
#include "sensors.h"

class ClimateController {
public:
    ClimateController();
    void init();
    void update();
    
    void setTargetTemperature(float temp);
    void setTargetHumidity(float humidity);
    float getTargetTemperature();
    float getTargetHumidity();
    
    void enableHeater();
    void disableHeater();
    void enableFan();
    void disableFan();
    
    bool isHeaterEnabled();
    bool isFanEnabled();
    
    float getCurrentTemperature();
    float getCurrentHumidity();
    
    // Метод для установки SensorManager
    void setSensorManager(SensorManager* sensorMgr);
    
private:
    float targetTemperature;
    float targetHumidity;
    float currentTemperature;
    float currentHumidity;
    
    bool heaterEnabled;
    bool fanEnabled;
    
    SensorManager* sensorManager;
};

#endif // CLIMATE_H