#ifndef DALLASTEMPERATURE_H
#define DALLASTEMPERATURE_H
#include <stdint.h>
#include "OneWire.h"
#define DEVICE_DISCONNECTED_C -127.0f
class DallasTemperature {
public:
    DallasTemperature(OneWire* ow) {}
    void begin() {}
    uint8_t getDeviceCount() { return 1; }
    void setResolution(uint8_t res) {}
    void setWaitForConversion(bool wait) {}
    void requestTemperatures() {}
    float getTempCByIndex(int idx);
};
#endif
