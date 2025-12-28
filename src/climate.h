#ifndef CLIMATE_H
#define CLIMATE_H

#include "config.h"

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
    
private:
    float targetTemperature;
    float targetHumidity;
    float currentTemperature;
    float currentHumidity;
    
    bool heaterEnabled;
    bool fanEnabled;
};

#endif // CLIMATE_H