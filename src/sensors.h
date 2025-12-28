#ifndef SENSORS_H
#define SENSORS_H

#include "config.h"

class SensorManager {
public:
    SensorManager();
    void init();
    void readSensors();
    
    float getTemperature();
    float getHumidity();
    float getOzoneLevel();
    
    bool isTemperatureValid();
    bool isHumidityValid();
    bool isOzoneValid();
    
private:
    float temperature;
    float humidity;
    float ozoneLevel;
    
    bool tempValid;
    bool humidityValid;
    bool ozoneValid;
};

#endif // SENSORS_H