#ifndef ADAFRUIT_HTU21DF_H
#define ADAFRUIT_HTU21DF_H
class Adafruit_HTU21DF {
public:
    bool begin() { return true; }
    float readTemperature();
    float readHumidity();
};
#endif
