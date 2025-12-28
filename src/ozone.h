#ifndef OZONE_H
#define OZONE_H

#include "config.h"

class OzoneController {
public:
    OzoneController();
    void init();
    void update();
    
    void enableOzoneGenerator();
    void disableOzoneGenerator();
    void enableUVLamp();
    void disableUVLamp();
    
    bool isOzoneGeneratorEnabled();
    bool isUVLampEnabled();
    
    void setOzoneLevel(float level);
    float getOzoneLevel();
    float getTargetOzoneLevel();
    
    void setOzoneInterval(unsigned long interval);
    unsigned long getOzoneInterval();
    
    void startOzoneCycle();
    void stopOzoneCycle();
    bool isOzoneCycleActive();
    
private:
    float targetOzoneLevel;
    float currentOzoneLevel;
    unsigned long ozoneInterval;
    unsigned long lastOzoneTime;
    
    bool ozoneGeneratorEnabled;
    bool uvLampEnabled;
    bool ozoneCycleActive;
};

#endif // OZONE_H