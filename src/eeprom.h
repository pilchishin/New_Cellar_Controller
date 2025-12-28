#ifndef EEPROM_H
#define EEPROM_H

#include "config.h"

class EEPROMManager {
public:
    EEPROMManager();
    void init();
    
    bool saveSettings();
    bool loadSettings();
    
    bool saveCalibrationData();
    bool loadCalibrationData();
    
    bool writeByte(int address, unsigned char value);
    unsigned char readByte(int address);
    
    bool writeFloat(int address, float value);
    float readFloat(int address);
    
    bool writeInt(int address, int value);
    int readInt(int address);
    
    void clearSettings();
    void clearCalibrationData();
    
private:
    bool settingsLoaded;
    bool calibrationLoaded;
};

#endif // EEPROM_H