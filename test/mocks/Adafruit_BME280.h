#ifndef ADAFRUIT_BME280_H
#define ADAFRUIT_BME280_H
#include <stdint.h>
class Adafruit_BME280 {
public:
    enum sensor_mode { MODE_NORMAL };
    enum sensor_sampling { SAMPLING_X1 };
    enum sensor_filter { FILTER_OFF };
    bool begin(uint8_t addr) { return true; }
    void setSampling(sensor_mode mode, sensor_sampling t, sensor_sampling p, sensor_sampling h, sensor_filter f) {}
    float readTemperature();
    float readHumidity();
    float readPressure() { return 1013.25f; }
};
#endif
