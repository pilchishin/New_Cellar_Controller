#ifndef UTILS_H
#define UTILS_H

#include "config.h"

class Utils {
public:
    static float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);
    static int constrainInt(int value, int min, int max);
    static float constrainFloat(float value, float min, float max);
    
    static bool isFloatEqual(float a, float b, float epsilon = 0.001f);
    static float roundToDecimal(float value, int decimals);
    
    static unsigned long calculateCRC(unsigned char* data, int length);
    static bool validateCRC(unsigned char* data, int length, unsigned long expectedCRC);
    
    static void delayMs(unsigned long ms);
    static unsigned long getSystemTime();
    
    static char* floatToString(float value, int decimals = 2);
    static char* intToString(int value);
    
    static bool stringToFloat(const char* str, float& result);
    static bool stringToInt(const char* str, int& result);
    
    static void debugPrint(const char* message);
    static void debugPrintln(const char* message);
    
    static void swap(float& a, float& b);
    static void swap(int& a, int& b);
    
    static float calculateMedian(float* array, int size);
    static float calculateAverage(float* array, int size);
    
private:
    Utils(); // Приватный конструктор для предотвращения создания экземпляров
};

#endif // UTILS_H