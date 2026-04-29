#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stddef.h>

#define HIGH 0x1
#define LOW  0x0
#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define A4 18
#define A5 19

#define WDTO_8S 8

typedef bool boolean;
typedef uint8_t byte;

#ifdef __cplusplus
extern "C" {
#endif
void setup();
void loop();

unsigned long millis();
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);

void wdt_enable(int timeout);
void wdt_reset();
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#define ARDUINO_MAX(a,b) ((a)>(b)?(a):(b))
#define ARDUINO_MIN(a,b) ((a)<(b)?(a):(b))
#endif

#define F(x) (const __FlashStringHelper*)(x)
#define PROGMEM
#define pgm_read_byte(addr) (*(const uint8_t*)(addr))
#define pgm_read_word(addr) (*(const uint16_t*)(addr))
#define pgm_read_ptr(addr) (*(const void**)(addr))
#define memcpy_P memcpy

class __FlashStringHelper;

class Print {
public:
    virtual size_t write(uint8_t) = 0;

    size_t print(const char* s) {
        size_t n = 0;
        if (!s) return 0;
        while (*s) n += write(*s++);
        return n;
    }

    size_t print(const __FlashStringHelper* s) {
        return print((const char*)s);
    }

    size_t print(int n) {
        char buf[12];
        sprintf(buf, "%d", n);
        return print(buf);
    }

    size_t print(unsigned int n) {
        char buf[12];
        sprintf(buf, "%u", n);
        return print(buf);
    }

    size_t print(unsigned long n) {
        char buf[22];
        sprintf(buf, "%lu", n);
        return print(buf);
    }

    size_t print(float f, int p = 2) {
        char fmt[10];
        sprintf(fmt, "%%.%df", p);
        char buf[32];
        sprintf(buf, fmt, f);
        return print(buf);
    }

    size_t println(const char* s) {
        size_t n = print(s);
        n += write('\n');
        return n;
    }

    size_t println(const __FlashStringHelper* s) {
        return println((const char*)s);
    }

    size_t println(int n) {
        size_t n_out = print(n);
        n_out += write('\n');
        return n_out;
    }

    size_t println(float f, int p = 2) {
        size_t n_out = print(f, p);
        n_out += write('\n');
        return n_out;
    }

    size_t println() {
        return write('\n');
    }
};

class SerialMock : public Print {
public:
    void begin(unsigned long speed) {}
    size_t write(uint8_t c) override { return putchar(c); }
};

extern SerialMock Serial;

#endif
