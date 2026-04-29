#include "Arduino.h"
#include "Wire.h"
#include "EEPROM.h"
#include "Simulator.h"
#include "Adafruit_BME280.h"
#include "Adafruit_HTU21DF.h"
#include "DallasTemperature.h"
#include "RTClib.h"
#include <chrono>
#include <thread>
#include <cstdlib>

unsigned long g_millis_offset = 0;
unsigned long g_millis_multiplier = 1;

CellarSimulator g_simulator;

unsigned long millis() {
    static auto start = std::chrono::steady_clock::now();
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count() * g_millis_multiplier + g_millis_offset;
}

void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms / g_millis_multiplier));
}

void delayMicroseconds(unsigned int us) {}

void pinMode(uint8_t pin, uint8_t mode) {}

int digitalRead(uint8_t pin) { return 1; } // HIGH = not pressed

void digitalWrite(uint8_t pin, uint8_t val) {
    if (pin == 6) { // PIN_RELAY_FAN
        g_simulator.SetFan(val == HIGH);
    }
    if (pin == 7) { // PIN_RELAY_OZONE
        g_simulator.SetOzone(val == HIGH);
    }
}

SerialMock Serial;

void wdt_enable(int timeout) {}
void wdt_reset() {}

TwoWire Wire;
EEPROMMock EEPROM;

// Sensor implementations
float Adafruit_BME280::readTemperature() {
    if (rand() % 500 == 0) return NAN; // Simulate occasional sensor read fail
    return g_simulator.GetInside().temp;
}
float Adafruit_BME280::readHumidity() { return g_simulator.GetInside().rh; }

float Adafruit_HTU21DF::readTemperature() { return g_simulator.GetOutside().temp; }
float Adafruit_HTU21DF::readHumidity() { return g_simulator.GetOutside().rh; }

float DallasTemperature::getTempCByIndex(int idx) { return g_simulator.GetInside().temp + 0.1f; }

DateTime RTC_DS3231::now() {
    unsigned long m = millis();
    uint32_t seconds = m / 1000;
    return DateTime(2023, 1, 1, (seconds / 3600) % 24, (seconds / 60) % 60, seconds % 60);
}
